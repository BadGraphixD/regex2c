#pragma once

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SKIP_WS(S) ((S) += strspn((S), " \n\t\r\f\v"))

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))
#define STRLITLEN(S) (ARRAY_SIZE(S) - 1)

typedef char bool_t;

typedef struct string {
  char *data;
  size_t length;
} string_t;

string_t create_string(char *data);
void append_char_to_str(string_t *string, char c);
void append_str_to_str(string_t *string, char *other);

const char *make_short_opts(const struct option *opts);
void print_options(int status, FILE *fout);
const char *opt_get_long(char short_opt);
const char *opt_format(char short_opt);
void opt_check_exclusive(unsigned char opt);
void opt_check_mutually_exclusive(unsigned char opt, const char *opts);

/**
 * Prints the given ASCII code {@code c} into a string. {@code -1} is
 * interpreted as EOF and printed as such. Printable characters are printed like
 * normal. Special characters are printed as such:
 * \0 for null character
 * \n for newline character
 * \r for carrier return
 * \t for tab
 * \s for space
 * \x__ for specifying the char hex code
 *
 * Any non-ASCII code is printed as:
 * \?________
 */
char *print_char(int c);
void fprint_indent(int indent, FILE *fout);
