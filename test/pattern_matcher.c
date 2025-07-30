#include <stdio.h>
#include <stdlib.h>

extern void parse();

void accept() {
  printf("\nAccept!\n");
  exit(EXIT_SUCCESS);
}

void reject() {
  printf("\nReject!\n");
  exit(EXIT_FAILURE);
}

int consume_next() { return getc(stdin); }

void checkpoint() {}

int main() { parse(); }
