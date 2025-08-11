#include "automaton2c.h"
#include "common.h"

#include <err.h>

void print_automaton_to_c_code(automaton_t automaton, char *parser_name,
                               char *next_name, char *acc_name, char *rej_name,
                               int flags, FILE *fout) {
  fprintf(fout, "%sint %s();\n", flags & 1 ? "static " : "", next_name);
  fprintf(fout, "%sint %s(int tag);\n", flags & 2 ? "static " : "", acc_name);
  fprintf(fout, "%svoid %s();\n", flags & 4 ? "static " : "", rej_name);
  fprintf(fout, "%svoid %s() {\n", flags & 8 ? "static " : "", parser_name);
  fprint_indent(2, fout);
  fprintf(fout, "int state = %d;\n", automaton.start_index);
  fprint_indent(2, fout);
  fprintf(fout, "while (1) {\n");

  bool_t *stm = create_state_transition_matrix(&automaton);

  fprint_indent(4, fout);
  fprintf(fout, "switch (state) {\n");

  for (int state = 0; state < automaton.max_node_count; state++) {
    fprint_indent(4, fout);
    fprintf(fout, "case %d:\n", state);
    int end_tag = automaton.nodes[state].end_tag;
    if (end_tag != -1) {
      // If the parsing could stop here, call accept with end tag
      fprint_indent(6, fout);
      fprintf(fout, "if (%s(%d)) { return; }\n", acc_name, end_tag);
    }
    fprint_indent(6, fout);
    fprintf(fout, "switch (%s()) {\n", next_name);

    for (int t = 0; t < 256; t++) {
      int range_start = t;
      int target = stm[state * 256 + t];

      if (target >= 0) {
        while (t + 1 < 256 && stm[state * 256 + (t + 1)] == target) {
          t++;
        }

        // Add transitions for all defined edges
        fprint_indent(6, fout);
        if (range_start == t) {
          fprintf(fout, "case %d:\n", t);
        } else {
          fprintf(fout, "case %d ... %d:\n", range_start, t);
        }
        fprint_indent(8, fout);
        fprintf(fout, "state = %d;\n", target);
        fprint_indent(8, fout);
        fprintf(fout, "continue;\n");
      }
    }

    // Reject if there is no transition for that terminal-state combo
    fprint_indent(6, fout);
    fprintf(fout, "default:\n");
    fprint_indent(8, fout);
    fprintf(fout, "%s();\n", rej_name);
    fprint_indent(8, fout);
    fprintf(fout, "return;\n");

    fprint_indent(6, fout);
    fprintf(fout, "}\n");
  }

  fprint_indent(4, fout);
  fprintf(fout, "}\n");

  fprint_indent(2, fout);
  fprintf(fout, "}\n");
  fprintf(fout, "}\n");
}
