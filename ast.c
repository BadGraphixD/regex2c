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

void print_ast_children(ast_t *ast, int indent, FILE *fout) {
  ast_child_list_t *children = ast->children;
  while (children) {
    print_ast_indented(&children->child, indent + 1, fout);
    children = children->next;
  }
}

void print_ast_indented(ast_t *ast, int indent, FILE *fout) {
  fprint_indent(indent, fout);
  switch (ast->type) {
  case OR_EXPR:
    fprintf(fout, "OR\n");
    print_ast_children(ast, indent, fout);
    break;
  case AND_EXPR:
    fprintf(fout, "AND\n");
    print_ast_children(ast, indent, fout);
    break;
  case CHAR:
    fprintf(fout, "CHAR\n");
    fprint_indent(indent + 1, fout);
    fprintf(fout, "%s\n", print_char(ast->terminal));
    break;
  case INV_CLASS:
    fprintf(fout, "INV_");
  case CLASS:
    fprintf(fout, "CLASS\n");
    fprint_indent(indent + 1, fout);
    for (int i = 0; i < 256; i++) {
      if (ast->terminals[i]) {
        fprintf(fout, "%s ", print_char(i));
      }
    }
    fprintf(fout, "\n");
    break;
  case STAR_MODIFIER:
    fprintf(fout, "STAR\n");
    print_ast_children(ast, indent, fout);
    break;
  case PLUS_MODIFIER:
    fprintf(fout, "PLUS\n");
    print_ast_children(ast, indent, fout);
    break;
  case OPT_MODIFIER:
    fprintf(fout, "OPT\n");
    print_ast_children(ast, indent, fout);
    break;
  case WILDCARD:
    fprintf(fout, "WILDCARD\n");
    break;
  case REFERENCE:
    fprintf(fout, "REFERENCE\n");
    if (ast->reference == NULL) {
      fprint_indent(indent + 1, fout);
      fprintf(fout, "NULL\n");
    } else {
      print_ast_indented(ast->reference, indent + 1, fout);
    }
    break;
  }
}

void print_ast(ast_t *ast, FILE *fout) { print_ast_indented(ast, 0, fout); }

void delete_ast_children(ast_t ast) {
  ast_child_list_t *children = ast.children;
  while (children != NULL) {
    ast_child_list_t *next = children->next;
    delete_ast(children->child);
    free(children);
    children = next;
  }
}

void delete_ast(ast_t ast) {
  switch (ast.type) {
  case CLASS:
  case INV_CLASS:
    free(ast.terminals);
    break;
  case OR_EXPR:
  case AND_EXPR:
  case STAR_MODIFIER:
  case PLUS_MODIFIER:
  case OPT_MODIFIER:
    delete_ast_children(ast);
    break;
  case CHAR:
  case WILDCARD:
  case REFERENCE:
    break;
  }
}
