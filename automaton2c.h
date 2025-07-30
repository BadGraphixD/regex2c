#pragma once

#include "automaton.h"

/*
 * Generates c code from the given {@code automaton}.
 *
 * The c code will define a function, which parses the string defined by the
 * automaton. The name of that function is defined by {@code parser_name}.
 *
 * This function relies on three functions, which will be called to fetch the
 * next character, accept, reject the string or set a checkpoint (a point at
 * which the parsing could stop and accept the string). The names of these
 * functions are defined by the parameters {@code next_name}, {@code acc_name},
 * {@code rej_name} and {@code checkpoint_name}.
 *
 * The implementations of these functions must be provided somewhere else (e.g.
 * in the code that the generated code will be linked with).
 */
void print_automaton_to_c_code(automaton_t automaton, char *parser_name,
                               char *next_name, char *acc_name, char *rej_name,
                               char *checkpoint_name);
