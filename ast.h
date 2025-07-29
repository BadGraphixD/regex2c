#pragma once

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
  REFERENCE,     // {DIGIT}
} ast_type_t;

typedef struct ast {
  ast_type_t type;
  union {
    struct ast_child_list *children;
    unsigned char *terminals;
    unsigned char terminal;
    struct ast *reference;
  };
} ast_t;

typedef struct ast_child_list {
  struct ast_child_list *next;
  ast_t child;
} ast_child_list_t;

void add_child(ast_t *ast, ast_t child);
void print_ast_indented(ast_t *ast, int indent);
void print_ast_children(ast_t *ast, int indent);
void print_ast_indented(ast_t *ast, int indent);
void print_ast(ast_t *ast);
void delete_ast(ast_t ast);
