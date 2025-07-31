#include <stdio.h>
#include <stdlib.h>

extern void parse();

int next_char = EOF;

int peek_next() { return next_char; }

int consume_next() {
  int c = peek_next();
  next_char = getc(stdin);
  return c;
}

int accept(int tag) {
  if (peek_next() == EOF) {
    // Only accept, if the whole string has been parsed
    printf("\nAccept!\n");
    exit(EXIT_SUCCESS);
  }
  printf("\nCheckpoint!\n");
  return 0;
}

void reject() {
  printf("\nReject!\n");
  exit(EXIT_FAILURE);
}

int main() {
  consume_next();
  parse();
}
