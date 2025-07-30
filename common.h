#pragma once

#include <stddef.h>

#define PRINT_DEBUG 0

typedef char bool_t;

typedef struct string {
  char *data;
  size_t length;
} string_t;

string_t create_string(char *data);
void append_char_to_str(string_t *string, char c);
void append_str_to_str(string_t *string, char *other);

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
void print_indent(int indent);
