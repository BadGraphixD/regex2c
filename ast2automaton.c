#include "ast2automaton.h"
#include "ast.h"

#include <stdlib.h>

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
  case REFERENCE:
    return get_automaton_nodes_from_ast(ast->reference);
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
    connect_nodes(automaton, *start, inner_start, 0, 1);
    connect_nodes(automaton, inner_end, *end, 0, 1);
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
    connect_nodes(automaton, new_end, old_start, 0, 1);
    children = children->next;
  }
}

void convert_ast_char_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                         int *start, int *end) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  connect_nodes(automaton, *start, *end, ast->terminal, 0);
}

void convert_ast_class_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                          int *start, int *end, int inverted) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  for (int i = 0; i < 256; i++) {
    if (ast->terminals[i] != inverted) {
      connect_nodes(automaton, *start, *end, i, 0);
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
  connect_nodes(automaton, inner_end, inner_start, 0, 1);
  connect_nodes(automaton, *start, inner_start, 0, 1);
  connect_nodes(automaton, inner_end, *end, 0, 1);
  if (is_star) {
    connect_nodes(automaton, *start, *end, 0, 1);
  }
}

void convert_ast_opt_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                        int *start, int *end) {
  convert_ast_to_automaton_nodes(automaton, &ast->children->child, start, end);
  connect_nodes(automaton, *start, *end, 0, 1);
}

void convert_ast_wildcard_to_automaton_nodes(automaton_t *automaton, ast_t *ast,
                                             int *start, int *end) {
  *start = create_node(automaton);
  *end = create_node(automaton);
  for (int i = 0; i < 256; i++) {
    connect_nodes(automaton, *start, *end, i, 0);
  }
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
  case REFERENCE:
    convert_ast_to_automaton_nodes(automaton, ast->reference, start, end);
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
