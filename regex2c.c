/**
 * We convert a regex input string (which is accepted via stdin) into c code,
 * which accepts any string which matches the regex string. The regex is truly
 * regular, so no backreferences, and also no capture groups.
 *
 * The generated c code looks something like this:
 *
 * void parse() {
 *   int state = 0;
 *   while (1) {
 *     switch (state) {
 *     case 0:
 *       switch (consume_next()) {
 *         case 97:
 *           state = 1;
 *           continue;
 *         default:
 *           reject();
 *           continue;
 *       }
 *     }
 *     case 1:
 *       switch (consume_next()) {
 *         case -1:
 *           accept();
 *           continue;
 *         default:
 *           reject();
 *           continue;
 *       }
 *     }
 *   }
 * }
 *
 * It depends on (but does not implement) the functions accept(), reject() and
 * consume_next(). The goal is also to generate a minimal automata (minimal c
 * code).
 *
 * @author Florian Malicky
 */

#include "ast2automaton.h"
#include "automaton2c.h"
#include "regex_parser.h"

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int next_char = EOF;
int char_pos = 0;

int peek_next() { return next_char; }

int consume_next() {
  int c = peek_next();
  next_char = getc(stdin);
  char_pos++;
  return c;
}

int reject(char *err, ...) {
  va_list args;
  va_start(args, err);
  char *errf = NULL;
  if (vasprintf(&errf, err, args) == -1) {
    errx(EXIT_FAILURE, "Failed to print error message");
  }
  va_end(args);
  errx(EXIT_FAILURE, "Rejected at char %d: %s", char_pos, errf);
}

ast_t *get_definition(char *name) { return NULL; }

extern bool_t is_end(int c) {
  switch (c) {
  case EOF:
  case '\n':
  case '\r':
  case '\t':
  case '\0':
  case ' ':
    return 1;
  default:
    return 0;
  }
}

int main() {
  consume_next();
  ast_t ast = consume_regex_expr();
  if (PRINT_DEBUG >= 1) {
    print_ast(&ast);
  }

  automaton_t automaton = convert_ast_to_automaton(&ast);
  delete_ast(ast);
  if (PRINT_DEBUG >= 1) {
    printf("NFA: ");
    print_automaton(&automaton);
  }

  automaton_t d_automaton = determinize(&automaton);
  delete_automaton(automaton);
  if (PRINT_DEBUG >= 1) {
    printf("DFA: ");
    print_automaton(&d_automaton);
  }

  automaton_t m_automaton = minimize(&d_automaton);
  delete_automaton(d_automaton);
  if (PRINT_DEBUG >= 1) {
    printf("Minimal DFA: ");
    print_automaton(&m_automaton);
  }

  if (PRINT_DEBUG == 0) {
    print_automaton_to_c_code(m_automaton, "parse", "consume_next", "accept",
                              "reject", 0);
  }

  delete_automaton(m_automaton);
}
