#include "regex_parser.h"

#include "ast.h"
#include "common.h"

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_REG_DEF_REF_LEN 1024

extern int peek_next();
extern int consume_next();
extern int reject(char *err, ...);
extern bool_t is_end(int c);
extern ast_t *get_definition(char *name);

int consume_hex_char() {
  int c = consume_next();
  if (c >= '0' && c <= '9') {
    return c - '0' + 0x0;
  }
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 0xa;
  }
  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 0xA;
  }
  reject("escaped hex char: unexpected char: '%s'", print_char(c));
  return 0;
}

int consume_char_() {
  if (peek_next() == '\\') {
    consume_next();
    switch (peek_next()) {
    case '[':
    case ']':
    case '(':
    case ')':
    case '.':
    case '-':
    case '^':
    case '|':
    case '*':
    case '+':
    case '?':
    case '\\':
      return consume_next();
    case '0':
      consume_next();
      return 0;
    case 't':
      consume_next();
      return 9;
    case 'n':
      consume_next();
      return 10;
    case 's':
      consume_next();
      return 32;
    case 'r':
      consume_next();
      return 13;
    case 'x':
      consume_next();
      int c = consume_hex_char();
      c += consume_hex_char() * 0x10;
      printf("Consumed char: %d\n", c);
      return c;
    default:
      // EOF and non-special chars cannot be escaped
      reject("escaped char: unexpected char after '\\': '%s'",
             print_char(peek_next()));
    }
  }
  switch (peek_next()) {
  case '[':
  case ']':
  case '(':
  case ')':
  case '.':
  case '-':
  case '^':
  case '|':
  case '*':
  case '+':
  case '?':
  case '\\':
    // If these characters appear unescaped, it's an error
    reject("unescaped char: unexpected (unescaped) char: '%s'",
           print_char(peek_next()));
  }
  switch (peek_next()) {
  case 0x21 ... 0x7e:
    // Includes alphanumerics and (some) special (printable) chars.
    // Special chars which are related to the regex syntax have been caughed
    // before.
    return consume_next();
  default:
    reject("unescaped char: unexpected char: '%s'", print_char(peek_next()));
    return 0;
  }
}

ast_t consume_char() {
  ast_t ast = {.type = CHAR, .terminal = consume_char_()};
  return ast;
}

ast_t consume_wildcard() {
  consume_next(); // consume '.'
  ast_t ast = {.type = WILDCARD};
  return ast;
}

void consume_char_or_range(unsigned char *terminals) {
  int c0 = consume_char_();
  if (peek_next() == '-') {
    consume_next();
    int c1 = consume_char_();
    if (c1 <= c0) {
      // start must be smaller than end
      reject("char range: start is not lower than end");
    }
    for (int c = c0; c < c1 + 1; c++) {
      if (c < 0 || c >= 256) {
        // this may be unnecessary, but better be safe
        reject("char range: invalid char encountered");
      }
      terminals[c] = 1;
    }
  } else {
    terminals[c0] = 1;
  }
}

ast_t consume_reference() {
  consume_next(); // consume '{'
  ast_t ast = {.type = REFERENCE, .reference = NULL};
  char name[MAX_REG_DEF_REF_LEN];
  int len = 0;
  while (1) {
    switch (peek_next()) {
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
    case '_':
      if (len >= MAX_REG_DEF_REF_LEN) {
        reject(
            "regular definition reference: name cannot be longer than %d chars",
            MAX_REG_DEF_REF_LEN);
      }
      name[len++] = consume_next();
      break;
    case '}':
      consume_next();
      name[len] = '\0';
      ast_t *ref = get_definition(name);
      if (ref == NULL) {
        reject("regular definition reference: regular definition with name "
               "does not exist: '%s'",
               name);
      }
      ast.reference = ref;
      return ast;
    default:
      reject("regular definition reference: unexpected char: '%s'",
             print_char(peek_next()));
    }
  }
}

ast_t consume_class() {
  consume_next(); // consume '['
  ast_t ast = {.type = CLASS, .terminals = calloc(256, sizeof(unsigned char))};
  if (peek_next() == '^') {
    consume_next();
    ast.type = INV_CLASS;
  }
  while (1) {
    consume_char_or_range(ast.terminals);
    switch (peek_next()) {
    case '[':
    case '(':
    case ')':
    case '.':
    case '-':
    case '^':
    case '|':
    case '*':
    case '+':
    case '?':
    case EOF:
      // these special chars can never be inside a class at this points
      reject("class: unexpected char: '%s'", print_char(peek_next()));
    case ']':
      consume_next();
      return ast;
    }
  }
}

ast_t consume_or_expr();

ast_t consume_parentheses() {
  consume_next(); // consume ')'
  ast_t ast = consume_or_expr();
  if (peek_next() != ')') {
    // missing closing ')'
    reject("parentheses: missing closing ')'");
  }
  consume_next();
  return ast;
}

ast_t consume_single() {
  switch (peek_next()) {
  case '{':
    return consume_reference();
  case '[':
    return consume_class();
  case '(':
    return consume_parentheses();
  case '.':
    return consume_wildcard();
  case ']':
  case ')':
  case '-':
  case '^':
  case '|':
  case '*':
  case '+':
  case '?':
  case EOF:
    reject("single: unexpected char: '%s'", print_char(peek_next()));
  default:
    return consume_char();
  }
}

ast_t make_modifier(ast_type_t modifier_type, ast_t child) {
  consume_next(); // consume the modifier char
  ast_t ast = {.type = modifier_type,
               .children = malloc(sizeof(ast_child_list_t))};
  ast.children->next = NULL;
  ast.children->child = child;
  return ast;
}

ast_t consume_modifier() {
  ast_t ast = consume_single();
  switch (peek_next()) {
  case '*':
    return make_modifier(STAR_MODIFIER, ast);
  case '+':
    return make_modifier(PLUS_MODIFIER, ast);
  case '?':
    return make_modifier(OPT_MODIFIER, ast);
  default:
    return ast;
  }
}

ast_t consume_and_expr() {
  ast_t ast = {.type = AND_EXPR, .children = NULL};
  int c = 0;
  while (1) {
    add_child(&ast, consume_modifier());
    c++;
    switch (peek_next()) {
    case ']':
    case '}':
    case '-':
    case '^':
    case '*':
    case '+':
    case '?':
      // this token can never come after a single
      reject("and expr: unexpected char: '%s'", print_char(peek_next()));
    case ')':
    case '|':
      if (c == 1) {
        return ast.children->child;
      }
      return ast;
    default:
      if (is_end(peek_next())) {
        if (c == 1) {
          return ast.children->child;
        }
        return ast;
      }
    }
  }
}

ast_t consume_or_expr() {
  ast_t ast = {.type = OR_EXPR, .children = NULL};
  int c = 0;
  while (1) {
    add_child(&ast, consume_and_expr());
    c++;
    if (peek_next() != '|') {
      if (c == 1) {
        return ast.children->child;
      }
      return ast;
    }
    consume_next();
  }
}

ast_t consume_regex_expr() {
  ast_t ast = consume_or_expr();
  if (!is_end(peek_next())) {
    reject("regex ex: unexpected char after expression: '%s' (expected ending "
           "character)",
           print_char(peek_next()));
  }
  return ast;
}
