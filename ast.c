#include "ast.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void add_child(ast_t *ast, ast_t child) {
  ast_child_list_t *list = malloc(sizeof(ast_child_list_t));
  list->next = ast->children;
  list->child = child;
  ast->children = list;
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
    printf("%s\n", print_char(ast->terminal));
    break;
  case INV_CLASS:
    printf("INV_");
  case CLASS:
    printf("CLASS\n");
    print_indent(indent + 1);
    for (int i = 0; i < 256; i++) {
      if (ast->terminals[i]) {
        printf("%s ", print_char(i));
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

void print_ast(ast_t *ast) { print_ast_indented(ast, 0); }

void delete_ast(ast_t ast) {
  if (ast.type == CLASS || ast.type == INV_CLASS) {
    free(ast.terminals);
  }
  ast_child_list_t *children = ast.children;
  while (children != NULL) {
    ast_child_list_t *next = children->next;
    delete_ast(children->child);
    free(children);
    children = next;
  }
}
