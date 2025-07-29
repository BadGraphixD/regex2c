#pragma once

#include "ast.h"

/**
 * Consumes all chars (using the functions {@code peek_next} and {@code
 * consume_next}) until EOF is reached. Tries to parse the consumed chars as a
 * regex string. Returns the AST of the parsed regular expression if successful.
 * If not, the function {@code reject} is called.
 *
 * This function expects the following functions to be implemented:
 *
 * int peek_next();
 * int consume_next();
 * int reject(char *err, ...);
 */
ast_t consume_regex_expr_until_eof();
