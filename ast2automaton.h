#pragma once

#include "ast.h"
#include "automaton.h"

automaton_t convert_ast_to_automaton(ast_t *ast);
