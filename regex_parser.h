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
 * bool_t is_end(int c);
 * ast_t *get_definition(char *name);
 *
 * peek_next      returns the next char, without consuming it
 * consume_next   returns and consumes the next char
 * reject         is called, when the given input sequence cannot be parsed
 * is_end         returns 1, when the given char ends the expression, or 0
 * get_definition returns the AST of the given regular definition, or NULL
 *
 * The regex syntax mostly follows today's conventions, with a few exceptions:
 *
 * There are no backreferences or capture groups.
 * The syntax for including a regular definitions is:
 * {@code ...{NAME_OF_THE_DEFINITION}...}
 * The names of definitions may contain alphanumerical letters and '_'
 *
 * The following special chars must be (always!) escaped in the regex:
 * ()[]{}+*?\.|^
 *
 * The following chars do not need to be escaped
 * 0-9a-zA-Z!"#$%&',/:;<=>@_`~
 *
 * The following escape codes exist:
 * \0 for null character
 * \n for newline character
 * \r for carrier return
 * \t for tab
 * \s for space
 * \x__ for specifying the char hex code
 *
 * All other characters are rejected!
 *
 */
ast_t consume_regex_expr();
