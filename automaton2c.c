#include "automaton2c.h"

#include <stdio.h>

void print_automaton_to_c_code(automaton_t automaton, char *parser_name,
                               char *next_name, char *acc_name,
                               char *rej_name) {
  printf("extern int %s();\n", next_name);
  printf("extern void %s();\n", acc_name);
  printf("extern void %s();\n", rej_name);
  printf("void %s() {\n", parser_name); // YES
  print_indent(2);
  printf("int state = %d;\n", automaton.start_index);
  print_indent(2);
  printf("while (1) {\n"); // YES

  bool_t *stm = create_state_transition_matrix(&automaton);

  print_indent(4);
  printf("switch (state) {\n"); // YES

  for (int state = 0; state < automaton.max_node_count; state++) {
    print_indent(4);
    printf("case %d:\n", state);
    print_indent(6);
    printf("switch (%s()) {\n", next_name); // YES

    for (int t = 0; t < 256; t++) {
      int range_start = t;
      int target = stm[state * 256 + t];

      if (target >= 0) {
        while (t + 1 < 256 && stm[state * 256 + (t + 1)] == target) {
          t++;
        }

        // Add transitions for all defined edges
        print_indent(8);
        if (range_start == t) {
          printf("case %d:\n", t);
        } else {
          printf("case %d ... %d:\n", range_start, t);
        }
        print_indent(10);
        printf("state = %d;\n", target);
        print_indent(10);
        printf("continue;\n");
      }
    }

    // When this state is an end state, EOF is accepted
    if (automaton.nodes[state].is_end) {
      print_indent(8);
      printf("case -1:\n");
      print_indent(10);
      printf("%s();\n", acc_name);
      print_indent(10);
      printf("continue;\n");
    }

    // Reject if there is no transition for that terminal-state combo
    print_indent(8);
    printf("default:\n");
    print_indent(10);
    printf("%s();\n", rej_name);
    print_indent(10);
    printf("continue;\n");

    print_indent(6);
    printf("}\n"); // YES
  }

  print_indent(4);
  printf("}\n"); // YES

  print_indent(2);
  printf("}\n"); // YES
  printf("}\n"); // YES
}
