#pragma once

#include <stdio.h>

#include "automaton.h"
#include "common.h"

#define REGEX2C_NEXT_DECL_STATIC 1
#define REGEX2C_ACCEPT_DECL_STATIC 2
#define REGEX2C_REJECT_DECL_STATIC 4
#define REGEX2C_PARSER_DECL_STATIC 8

#define REGEX2C_ALL_DECL_STATIC 15

/*
 * Generates c code from the given {@code automaton}.
 *
 * The c code will define a function, which parses the string defined by the
 * automaton. The name of that function is defined by {@code parser_name}.
 *
 * This function relies on three functions, which will be called to fetch the
 * next character, accept or reject the strin. Accept is called when the parsing
 * reaches an end node and the tag of the end node is supplied as the first
 * argument. If accept returns 0, the parsing will continue (until the next end
 * point is reached or reject is called). If it returns 1, the parser returns
 * immediately. The names of the functions are defined by the parameters {@code
 * next_name}, {@code acc_name}, and {@code rej_name}.
 *
 * {@code flags} is used to further configure the generated c code.
 *
 * The implementations of these functions must be provided somewhere else (e.g.
 * in the code that the generated code will be linked with).
 */
void print_automaton_to_c_code(automaton_t automaton, char *parser_name,
                               char *next_name, char *acc_name, char *rej_name,
                               int flags, FILE *fout);
