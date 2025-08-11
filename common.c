#include "common.h"

#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPT_ARG_STR "[=ARG]"
#define REQ_ARG_STR "=ARG"

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

extern const struct option OPTIONS_LONG[];
extern const char *const OPTIONS_HELP[];
extern const char *prog_name;
extern bool_t opts_given[];

const char *make_short_opts(const struct option *opts) {
  size_t len = 1;
  for (const struct option *opt = opts; opt->name != NULL; opt++) {
    len += 1 + (unsigned)opt->has_arg;
  }

  char *short_opts = malloc(len + 1);
  char *s = short_opts;

  *s++ = ':';
  for (const struct option *opt = opts; opt->name != NULL; opt++) {
    *s++ = (char)opt->val;
    switch (opt->has_arg) {
    case optional_argument:
      *s++ = ':';
    case required_argument:
      *s++ = ':';
    }
  }
  *s++ = '\0';

  return short_opts;
}

_Noreturn void usage(int status) {
  size_t longest_opt_len = 0;
  for (const struct option *opt = OPTIONS_LONG; opt->name != NULL; opt++) {
    size_t opt_len = strlen(opt->name);
    switch (opt->has_arg) {
    case no_argument:
      break;
    case optional_argument:
      opt_len += STRLITLEN(OPT_ARG_STR);
      break;
    case required_argument:
      opt_len += STRLITLEN(REQ_ARG_STR);
      break;
    }
    if (opt_len > longest_opt_len) {
      longest_opt_len = opt_len;
    }
  }

  FILE *fout = status == 0 ? stdout : stderr;
  fprintf(fout, "usage: %s [options] ...\noptions:\n", prog_name);

  for (const struct option *opt = OPTIONS_LONG; opt->name != NULL; opt++) {
    fprintf(fout, "  --%s", opt->name);
    size_t opt_len = strlen(opt->name);
    switch (opt->has_arg) {
    case no_argument:
      break;
    case optional_argument:
      opt_len += fprintf(fout, OPT_ARG_STR);
      break;
    case required_argument:
      opt_len += fprintf(fout, REQ_ARG_STR);
      break;
    }
    fprintf(fout, "%*s (-%c) %s.\n", (int)(longest_opt_len - opt_len), "",
            opt->val, OPTIONS_HELP[opt->val]);
  }

  exit(status);
}

const char *opt_get_long(char short_opt) {
  for (const struct option *opt = OPTIONS_LONG; opt->name != NULL; opt++) {
    if (opt->val == short_opt) {
      return opt->name;
    }
  }
  return "";
}

const char *opt_format(char short_opt) {
  static char bufs[2][32];
  static unsigned buf_index;
  char *const buf = bufs[buf_index++];
  const char *const long_opt = opt_get_long(short_opt);
  snprintf(buf, 32, "%s%s%s-%c", long_opt[0] != '\0' ? "--" : "", long_opt,
           long_opt[0] != '\0' ? "/" : "", short_opt);
  return buf;
}

static unsigned char min_valid_opt() {
  unsigned char min = 255;
  for (const struct option *opt = OPTIONS_LONG; opt->name != NULL; opt++) {
    if (opt->val > min) {
      min = opt->val;
    }
  }
  return min;
}

static unsigned char max_valid_opt() {
  unsigned char max = 0;
  for (const struct option *opt = OPTIONS_LONG; opt->name != NULL; opt++) {
    if (opt->val > max) {
      max = opt->val;
    }
  }
  return max;
}

void opt_check_exclusive(unsigned char opt) {
  if (!opts_given[opt]) {
    return;
  }
  unsigned char max = max_valid_opt();
  for (unsigned char i = min_valid_opt(); i < max; i++) {
    if (i == opt) {
      continue;
    }
    if (opts_given[i]) {
      errx(EXIT_FAILURE, "%s can be given only by itself\n", opt_format(opt));
    }
  }
}

void opt_check_mutually_exclusive(unsigned char opt, const char *opts) {
  if (!opts_given[opt]) {
    return;
  }
  for (; *opts != '\0'; opts++) {
    if (opts_given[(unsigned)*opts]) {
      errx(EXIT_FAILURE, "%s and %s are mutually exclusive\n", opt_format(opt),
           opt_format(*opts));
    }
  }
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

void fprint_indent(int indent, FILE *fout) {
  while (indent-- > 0) {
    fprintf(fout, " ");
  }
}
