#include "common.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string_t create_string(char *data) {
  if (data == NULL) {
    string_t string = {.data = malloc(sizeof(char)), .length = 0};
    string.data[0] = '\0';
    return string;
  }
  size_t len = strlen(data);
  string_t string = {.data = malloc((len + 1) * sizeof(char)), .length = len};
  memcpy(string.data, data, len + 1);
  return string;
}

void append_char_to_str(string_t *string, char c) {
  string->length++;
  string->data = realloc(string->data, (string->length + 1) * sizeof(char));
  string->data[string->length - 1] = c;
  string->data[string->length] = '\0';
}

void append_str_to_str(string_t *string, char *other) {
  size_t old_len = string->length;
  size_t other_len = strlen(other);
  string->length += other_len;
  string->data = realloc(string->data, (string->length + 1) * sizeof(char));
  memcpy(&string->data[old_len], other, other_len + 1);
}

char *print_char(int c) {
  char *result = NULL;
  switch (c) {
  case EOF:
    return "EOF";
  case 0:
    return "\\0";
  case 9:
    return "\\t";
  case 10:
    return "\\n";
  case 13:
    return "\\r";
  case 32:
    return "\\s";
  case 33 ... 126:
    asprintf(&result, "%c", c);
    break;
  default:
    if (c >= 0 && c < 256) {
      asprintf(&result, "\\x%hhx", c);
    } else {
      asprintf(&result, "\\?%x", c);
    }
  }
  return result;
}

void print_indent(int indent) {
  while (indent-- > 0) {
    printf(" ");
  }
}
