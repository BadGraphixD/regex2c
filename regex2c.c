/**
 * We convert a regex input string (which is accepted via stdin) into c code,
 * which accepts any string which matches the regex string. The regex is truly
 * regular, so no backreferences, and also no capture groups.
 *
 * The following special chars must be escaped in the regex:
 * ()[]+*?\.|^
 *
 * The following chars do not need to be escaped
 * 0-9a-zA-Z!"#$%&',/:;<=>@_`{}~
 *
 * The following escape codes exist:
 * \0 for null character
 * \n for newline character
 * \r for carrier return
 * \t for tab
 * \s for space
 * \x__ for specifying the char hex code
 *
 * All other characters are rejected!
 *
 * The generated c code looks something like this:
 *
 * void parse() {
 *   switch (next_char()) {
 *     case '0' ... '9':
 *       accept();
 *     default:
 *       reject();
 *   }
 * }
 *
 * It depends on (but does not implement) the functions accept(), reject() and
 * next_char(). The goal is also to generate a minimal automata (minimal c
 * code).
 *
 * @author Florian Malicky
 */

#include "ast.h"
#include "automaton.h"
#include "common.h"

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int next_char;

int peek_next() { return next_char; }

int consume_next() {
  int c = peek_next();
  next_char = getc(stdin);
  return c;
}

int reject(char *nt, char *err, ...) {
  va_list args;
  va_start(args, err);
  char *errf = NULL;
  if (vasprintf(&errf, err, args) == -1) {
    errx(EXIT_FAILURE, "Failed to print error message");
  }
  va_end(args);
  errx(EXIT_FAILURE, "Rejected while parsing %s: %s", nt, errf);
}

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
  reject("escaped hex char", "unexpected char: '%s'", print_char(c));
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
      reject("escaped char", "unexpected char after '\\': '%s'",
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
    reject("unescaped char", "unexpected (unescaped) char: '%s'",
           print_char(peek_next()));
  }
  switch (peek_next()) {
  case 0x21 ... 0x7e:
    // Includes alphanumerics and (some) special (printable) chars.
    // Special chars which are related to the regex syntax have been caughed
    // before.
    return consume_next();
  default:
    reject("unescaped char", "unexpected char: '%s'", print_char(peek_next()));
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
      reject("char range", "start is not lower than end");
    }
    for (int c = c0; c < c1 + 1; c++) {
      if (c < 0 || c >= 256) {
        // this may be unnecessary, but better be safe
        reject("char range", "invalid char encountered");
      }
      terminals[c] = 1;
    }
  } else {
    terminals[c0] = 1;
  }
}

ast_t consume_class() {
  consume_next(); // consume '['
  ast_t ast = {.type = CLASS, .terminals = calloc(256, sizeof(char))};
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
      reject("class", "unexpected char: '%s'", print_char(peek_next()));
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
    reject("parentheses", "missing closing ')'");
  }
  consume_next();
  return ast;
}

ast_t consume_single() {
  switch (peek_next()) {
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
    reject("single", "unexpected char: '%s'", print_char(peek_next()));
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
    case '-':
    case '^':
    case '*':
    case '+':
    case '?':
      // this token can never come after a single
      reject("and expr", "unexpected char: '%s'", print_char(peek_next()));
    case ')':
    case '|':
    case EOF:
      if (c == 1) {
        return ast.children->child;
      }
      return ast;
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
  if (peek_next() != EOF) {
    reject("regex expr",
           "unexpected char after expression: '%s' (expected EOF)",
           print_char(peek_next()));
  }
  return ast;
}

int get_automaton_nodes_from_ast(ast_t *ast);

int get_automaton_nodes_from_ast_children(ast_t *ast) {
  int c = 0;
  ast_child_list_t *list = ast->children;
  while (list != NULL) {
    c += get_automaton_nodes_from_ast(&list->child);
    list = list->next;
  }
  return c;
}

int get_automaton_nodes_from_ast(ast_t *ast) {
  switch (ast->type) {
  case OR_EXPR:
  case STAR_MODIFIER:
  case PLUS_MODIFIER:
    // These need to add an extra 2 nodes surrounding the inner automaton, for
    // epsilon-transitions
    return 2 + get_automaton_nodes_from_ast_children(ast);
  case AND_EXPR:
  case OPT_MODIFIER:
    // These do not need extra nodes
    return get_automaton_nodes_from_ast_children(ast);
  case CHAR:
  case CLASS:
  case INV_CLASS:
  case WILDCARD:
    // These convert to exactly 2 nodes, and a bunch of connections between them
    return 2;
  }
  return 0;
}

void convert_ast_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                    int *start, int *end);

void convert_ast_or_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                       int *start, int *end) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  ast_child_list_t *list = ast->children;
  while (list != NULL) {
    int inner_start, inner_end;
    convert_ast_to_automaton_nodes(automaton, &list->child, &inner_start,
                                   &inner_end);
    connect_nodes(automaton, *start, inner_start, 0, 1, 0);
    connect_nodes(automaton, inner_end, *end, 0, 1, 0);
    list = list->next;
  }
}

void convert_ast_and_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                        int *start, int *end) {
  ast_child_list_t *children = ast->children;
  convert_ast_to_automaton_nodes(automaton, &children->child, start, end);
  children = children->next;
  while (children != NULL) {
    // Prepend, because and-children are stored in reverse in ast
    int old_start = *start;
    int new_end;
    convert_ast_to_automaton_nodes(automaton, &children->child, start,
                                   &new_end);
    connect_nodes(automaton, new_end, old_start, 0, 1, 0);
    children = children->next;
  }
}

void convert_ast_char_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                         int *start, int *end) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  connect_nodes(automaton, *start, *end, ast->terminal, 0, 0);
}

void convert_ast_class_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                          int *start, int *end, int inverted) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  for (int i = 0; i < 256; i++) {
    if (ast->terminals[i] != inverted) {
      connect_nodes(automaton, *start, *end, i, 0, 0);
    }
  }
}

void convert_ast_star_plus_to_automaton_nodes(automaton_t *automaton,
                                              ast_t *ast, int *start, int *end,
                                              int is_star) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  int inner_start, inner_end;
  convert_ast_to_automaton_nodes(automaton, &ast->children->child, &inner_start,
                                 &inner_end);
  connect_nodes(automaton, inner_end, inner_start, 0, 1, 0);
  connect_nodes(automaton, *start, inner_start, 0, 1, 0);
  connect_nodes(automaton, inner_end, *end, 0, 1, 0);
  if (is_star) {
    connect_nodes(automaton, *start, *end, 0, 1, 0);
  }
}

void convert_ast_opt_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                        int *start, int *end) {
  convert_ast_to_automaton_nodes(automaton, &ast->children->child, start, end);
  connect_nodes(automaton, *start, *end, 0, 1, 0);
}

void convert_ast_wildcard_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                             int *start, int *end) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  connect_nodes(automaton, *start, *end, 0, 0, 1);
}

void convert_ast_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                    int *start, int *end) {
  switch (ast->type) {
  case OR_EXPR:
    convert_ast_or_to_automaton_nodes(automaton, ast, start, end);
    return;
  case AND_EXPR:
    convert_ast_and_to_automaton_nodes(automaton, ast, start, end);
    return;
  case CHAR:
    convert_ast_char_to_automaton_nodes(automaton, ast, start, end);
    return;
  case CLASS:
    convert_ast_class_to_automaton_nodes(automaton, ast, start, end, 0);
    return;
  case INV_CLASS:
    convert_ast_class_to_automaton_nodes(automaton, ast, start, end, 1);
    return;
  case STAR_MODIFIER:
    convert_ast_star_plus_to_automaton_nodes(automaton, ast, start, end, 1);
    return;
  case PLUS_MODIFIER:
    convert_ast_star_plus_to_automaton_nodes(automaton, ast, start, end, 0);
    return;
  case OPT_MODIFIER:
    convert_ast_opt_to_automaton_nodes(automaton, ast, start, end);
    return;
  case WILDCARD:
    convert_ast_wildcard_to_automaton_nodes(automaton, ast, start, end);
    return;
  }
}

automaton_t convert_ast_to_automaton(ast_t *ast) {
  automaton_t automaton = create_automaton(get_automaton_nodes_from_ast(ast));
  int start, end;
  convert_ast_to_automaton_nodes(&automaton, ast, &start, &end);
  automaton.start_index = start;
  automaton.nodes[end].is_end = 1;
  return automaton;
}

int main() {
  consume_next();
  ast_t ast = consume_regex_expr();
  print_ast(&ast);
  automaton_t automaton = convert_ast_to_automaton(&ast);
  print_automaton(&automaton);
}
