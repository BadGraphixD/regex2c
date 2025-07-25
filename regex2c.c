/**
 * We convert a regex input string (which is accepted via stdin) into c code,
 * which accepts any string which matches the regex string. The regex is truly
 * regular, so no backreferences, and also no capture groups.
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum ast_type {
  OR_EXPR,       // a|b
  AND_EXPR,      // abc
  CHAR,          // a
  CLASS,         // [a-z]
  INV_CLASS,     // [^a-z]
  STAR_MODIFIER, // a*
  PLUS_MODIFIER, // a+
  OPT_MODIFIER,  // a?
  WILDCARD,      // .
} ast_type_t;

typedef struct ast {
  ast_type_t type;
  struct ast_child_list *children;
  union {
    char *triggers;
    char trigger;
  };
} ast_t;

typedef struct ast_child_list {
  struct ast_child_list *next;
  ast_t child;
} ast_child_list_t;

void add_child(ast_t *ast, ast_t child) {
  ast_child_list_t *list = malloc(sizeof(ast_child_list_t));
  list->next = ast->children;
  list->child = child;
  ast->children = list;
}

void print_indent(int indent) {
  while (indent-- > 0) {
    printf(" ");
  }
}

void print_ast_indented(ast_t *ast, int indent);

void print_ast_children(ast_t *ast, int indent) {
  ast_child_list_t *children = ast->children;
  while (children) {
    print_ast_indented(&children->child, indent + 1);
    children = children->next;
  }
}

void print_ast_indented(ast_t *ast, int indent) {
  print_indent(indent);
  switch (ast->type) {
  case OR_EXPR:
    printf("OR\n");
    print_ast_children(ast, indent);
    break;
  case AND_EXPR:
    printf("AND\n");
    print_ast_children(ast, indent);
    break;
  case CHAR:
    printf("CHAR\n");
    print_indent(indent + 1);
    printf("%c\n", ast->trigger);
    break;
  case INV_CLASS:
    printf("INV_");
  case CLASS:
    printf("CLASS\n");
    print_indent(indent + 1);
    for (int i = 0; i < 256; i++) {
      if (ast->triggers[i]) {
        printf("%c ", i);
      }
    }
    printf("\n");
    break;
  case STAR_MODIFIER:
    printf("STAR\n");
    print_ast_children(ast, indent);
    break;
  case PLUS_MODIFIER:
    printf("PLUS\n");
    print_ast_children(ast, indent);
    break;
  case OPT_MODIFIER:
    printf("OPT\n");
    print_ast_children(ast, indent);
    break;
  case WILDCARD:
    printf("WILDCARD\n");
    break;
  }
}

void print_ast(ast_t ast) { print_ast_indented(&ast, 0); }

int next_char;

int peek_next() { return next_char; }

int consume_next() {
  int c = peek_next();
  next_char = getc(stdin);
  return c;
}

int reject(char *nt, char *err) {
  errx(EXIT_FAILURE, "Rejected while parsing %s: %s", nt, err);
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
      break;
    default:
      // EOF and non-special chars cannot be escaped
      reject("escaped char", "unexpected char after '\\'");
    }
  }
  return consume_next();
}

ast_t consume_char() {
  ast_t ast = {.type = CHAR, .trigger = consume_char_()};
  return ast;
}

ast_t consume_wildcard() {
  consume_next(); // consume '.'
  ast_t ast = {.type = WILDCARD};
  return ast;
}

void consume_char_or_range(char *triggers) {
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
      triggers[c] = 1;
    }
  } else {
    triggers[c0] = 1;
  }
}

ast_t consume_class() {
  consume_next(); // consume '['
  ast_t ast = {.type = CLASS, .triggers = calloc(256, sizeof(char))};
  if (peek_next() == '^') {
    consume_next();
    ast.type = INV_CLASS;
  }
  while (1) {
    consume_char_or_range(ast.triggers);
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
      reject("class", "unexpected special char or EOF");
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
    reject("single", "unexpected special char");
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
  while (1) {
    add_child(&ast, consume_modifier());
    switch (peek_next()) {
    case ']':
    case '.':
    case '-':
    case '^':
    case '*':
    case '+':
    case '?':
      // this token can never come after a single
      reject("and expr", "unexpected special char");
    case ')':
    case '|':
    case EOF:
      return ast;
    }
  }
}

ast_t consume_or_expr() {
  ast_t ast = {.type = OR_EXPR, .children = NULL};
  while (1) {
    add_child(&ast, consume_and_expr());
    if (peek_next() != '|') {
      return ast;
    }
    consume_next();
  }
}

ast_t consume_regex_expr() {
  ast_t ast = consume_or_expr();
  if (peek_next() != EOF) {
    reject("regex expr", "unexpected char after expression (expected EOF)");
  }
  return ast;
}

int main() {
  consume_next();
  ast_t ast = consume_regex_expr();
  print_ast(ast);
}
