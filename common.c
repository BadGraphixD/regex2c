#include "common.h"

#include <stdio.h>

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
