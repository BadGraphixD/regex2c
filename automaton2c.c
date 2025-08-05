#include "automaton2c.h"
#include "common.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

void print_decl_modifier(int flags) {
  bool_t static_decl = flags & 1;
  bool_t extern_decl = flags & 2;
  if (static_decl && extern_decl) {
    errx(EXIT_FAILURE,
         "Invalid flags: cannot declare c function as both static and extern");
  }
  if (static_decl) {
    printf("static ");
  }
  if (extern_decl) {
    printf("extern ");
  }
}

void print_automaton_to_c_code(automaton_t automaton, char *parser_name,
                               char *next_name, char *acc_name, char *rej_name,
                               int flags) {
  print_decl_modifier(flags << 0);
  printf("int %s();\n", next_name);

  print_decl_modifier(flags << 2);
  printf("int %s(int tag);\n", acc_name);

  print_decl_modifier(flags << 4);
  printf("void %s();\n", rej_name);

  print_decl_modifier(flags << 6);
  printf("void %s() {\n", parser_name);
  print_indent(2);
  printf("int state = %d;\n", automaton.start_index);
  print_indent(2);
  printf("while (1) {\n");

  bool_t *stm = create_state_transition_matrix(&automaton);

  print_indent(4);
  printf("switch (state) {\n");

  for (int state = 0; state < automaton.max_node_count; state++) {
    print_indent(4);
    printf("case %d:\n", state);
    int end_tag = automaton.nodes[state].end_tag;
    if (end_tag != -1) {
      // If the parsing could stop here, call accept with end tag
      print_indent(6);
      printf("if (%s(%d)) { return; }\n", acc_name, end_tag);
    }
    print_indent(6);
    printf("switch (%s()) {\n", next_name);

    for (int t = 0; t < 256; t++) {
      int range_start = t;
      int target = stm[state * 256 + t];

      if (target >= 0) {
        while (t + 1 < 256 && stm[state * 256 + (t + 1)] == target) {
          t++;
        }

        // Add transitions for all defined edges
        print_indent(6);
        if (range_start == t) {
          printf("case %d:\n", t);
        } else {
          printf("case %d ... %d:\n", range_start, t);
        }
        print_indent(8);
        printf("state = %d;\n", target);
        print_indent(8);
        printf("continue;\n");
      }
    }

    // Reject if there is no transition for that terminal-state combo
    print_indent(6);
    printf("default:\n");
    print_indent(8);
    printf("%s();\n", rej_name);
    print_indent(8);
    printf("return;\n");

    print_indent(6);
    printf("}\n");
  }

  print_indent(4);
  printf("}\n");

  print_indent(2);
  printf("}\n");
  printf("}\n");
}
