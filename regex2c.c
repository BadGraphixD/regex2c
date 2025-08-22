/**
 * We convert a regular expression into an automata in c code, which accepts any
 * string which matches the regular expression.
 *
 * The generated c code looks something like this:
 *
 * int consume_next();
 * int accept(int tag);
 * void reject();
 * void parse() {
 *   int state = 0;
 *   while (1) {
 *     switch (state) {
 *     case 0:
 *       switch (consume_next()) {
 *       case 97:
 *         state = 1;
 *         continue;
 *       default:
 *         reject();
 *         return;
 *       }
 *     case 1:
 *       switch (consume_next()) {
 *       case 98:
 *         state = 2;
 *         continue;
 *       default:
 *         reject();
 *         return;
 *       }
 *     case 2:
 *       if (accept(0)) { return; }
 *       switch (consume_next()) {
 *       default:
 *         reject();
 *         return;
 *       }
 *     }
 *   }
 * }
 *
 * This example was generated from the regular expression "ab".
 *
 * It depends on (but does not implement) the functions accept, reject and
 * consume_next. The goal is also to generate a minimal automata (but not
 * minimal c code).
 *
 * @author Florian Malicky
 */

#include "ast2automaton.h"
#include "automaton2c.h"
#include "common.h"
#include "not_enough_cli/not_enough_cli.h"
#include "regex_parser.h"

#include <err.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int next_char = EOF;
static int char_pos = 0;

static char **in_files = NULL;
static int fin_idx = 0;
static FILE *fin = NULL;

int peek_next() { return next_char; }

static void open_next_in_file() {
  if (fin != NULL) {
    fclose(fin);
  }
  char *next_in_file_name = in_files[fin_idx++];
  if (next_in_file_name == NULL) {
    fin = NULL;
    return;
  }
  if (strcmp(next_in_file_name, "-") == 0) {
    fin = stdin;
  } else {
    fin = fopen(next_in_file_name, "r");
    if (fin == NULL) {
      errx(EXIT_FAILURE, "Cannot open file \"%s\"\n", next_in_file_name);
    }
  }
}

static int get_next_input_char() {
  if (in_files == NULL) {
    return getc(stdin);
  } else {
    if (fin == NULL) {
      return EOF;
    }
    int next = getc(fin);
    if (next == EOF) {
      open_next_in_file();
      return get_next_input_char();
    }
    return next;
  }
}

int consume_next() {
  int c = peek_next();
  next_char = get_next_input_char();
  char_pos++;
  return c;
}

_Noreturn int reject(char *err, ...) {
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

bool_t is_end(int c) {
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

struct option OPTIONS_LONG[] = {{"help", no_argument, NULL, 'h'},
                                {"version", no_argument, NULL, 'v'},
                                {"debug", no_argument, NULL, 'd'},
                                {"output", required_argument, NULL, 'o'},
                                {NULL, 0, NULL, 0}};

static char *OPTIONS_HELP[] = {
    ['h'] = "print this help list",
    ['v'] = "print program version",
    ['d'] = "output debug information",
    ['o'] = "set output file name",
};

static char *out_file_name = NULL;
static FILE *out_file = NULL;
static bool_t output_debug_info = 0;

_Noreturn static void version() {
  printf("regex2c 1.0\n");
  exit(EXIT_SUCCESS);
}

_Noreturn static void usage(int status) {
  FILE *fout = status == 0 ? stdout : stderr;
  nac_print_usage_header(fout, "[OPTION]... [FILE]...");
  fprintf(fout, "Convert a regular expression into an automata in c-code.\n\n");
  fprintf(fout, "With no FILE, or when FILE is -, read standard input.\n\n");
  nac_print_options(fout);
  exit(status);
}

static void handle_option(char opt) {
  switch (opt) {
  case 'o':
    out_file_name = nac_optarg_trimmed();
    if (out_file_name[0] == '\0') {
      nac_missing_arg('o');
    }
    break;
  case 'd':
    output_debug_info = 1;
    break;
  }
}

static void parse_args(int *argc, char ***argv) {
  nac_set_opts(**argv, OPTIONS_LONG, OPTIONS_HELP);
  nac_simple_parse_args(argc, argv, handle_option);

  nac_opt_check_excl("hv");
  nac_opt_check_max_once("hvo");

  if (nac_get_opt('h')) {
    usage(*argc > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
  }
  if (nac_get_opt('v')) {
    if (*argc > 0) {
      usage(EXIT_FAILURE);
    }
    version();
  }

  if (out_file_name == NULL) {
    out_file = stdout;
  } else {
    out_file = fopen(out_file_name, "w");
    if (out_file == NULL) {
      errx(EXIT_FAILURE, "Failed to open specified output file \"%s\"\n",
           out_file_name);
    }
  }

  if (*argc > 0) {
    in_files = malloc(sizeof(char *) * (*argc + 1));
    for (int i = 0; i < *argc; i++) {
      in_files[i] = (*argv)[i];
    }
    in_files[*argc] = NULL;
    open_next_in_file();
  }

  nac_cleanup();
}

int main(int argc, char **argv) {
  parse_args(&argc, &argv);
  consume_next();
  ast_t ast = consume_regex_expr();
  if (output_debug_info) {
    fprintf(out_file, "--- Abstract syntax tree:\n");
    print_ast(&ast, out_file);
    fprintf(out_file, "\n");
  }

  automaton_t automaton = convert_ast_to_automaton(&ast);
  delete_ast(ast);
  if (output_debug_info) {
    fprintf(out_file, "--- NFA:\n");
    print_automaton(&automaton, out_file);
    fprintf(out_file, "\n");
  }

  automaton_t d_automaton = determinize(&automaton);
  delete_automaton(automaton);
  if (output_debug_info) {
    fprintf(out_file, "--- DFA:\n");
    print_automaton(&d_automaton, out_file);
    fprintf(out_file, "\n");
  }

  automaton_t m_automaton = minimize(&d_automaton);
  delete_automaton(d_automaton);
  if (output_debug_info) {
    fprintf(out_file, "--- Minimal DFA:\n");
    print_automaton(&m_automaton, out_file);
    fprintf(out_file, "\n--- C code:\n");
  }

  print_automaton_to_c_code(m_automaton, "parse", "consume_next", "accept",
                            "reject", 0, out_file);

  delete_automaton(m_automaton);
}
