#pragma once

#include "ast.h"
#include "automaton.h"

typedef struct ast_list {
  struct ast_list *next;
  ast_t *ast;
} ast_list_t;

/**
 * Converts the given {@code ast} to an automaton. The end node is tagged with
 * {@code 0}.
 */
automaton_t convert_ast_to_automaton(ast_t *ast);

/**
 * Converts the given {@code ast_list} to an automaton. Each ast is converted to
 * an automaton and these automata are connected via an or-construct. Each
 * original ast's end node gets a different end tag, starting with {@code 0} and
 * incrementing.
 */
automaton_t convert_ast_list_to_automaton(ast_list_t *ast_list);
