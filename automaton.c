#include "automaton.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>

int edge_idx(automaton_t *automaton, int node0, int node1) {
  return node1 * automaton->max_node_count + node0;
}

void print_automaton(automaton_t *automaton) {
  printf("Automaton (max nodes = %d, node count = %d)\n",
         automaton->max_node_count, automaton->next_node_index);

  for (int node0 = 0; node0 < automaton->max_node_count; node0++) {
    printf("%c%c #%d", automaton->start_index == node0 ? '>' : ' ',
           automaton->nodes[node0].is_end ? '*' : ' ', node0);

    for (int node1 = 0; node1 < automaton->max_node_count; node1++) {
      int edge = edge_idx(automaton, node0, node1);

      if (automaton->adjacency_matrix[edge].transitions[EPSILON_EDGE]) {
        printf(" ε->%d", node1);
      }

      int is_wildcard = 1;
      for (int t = 0; t < 256; t++) {
        if (!automaton->adjacency_matrix[edge].transitions[t]) {
          is_wildcard = 0;
          break;
        }
      }

      if (is_wildcard) {
        printf(" any->%d", node1);
      } else {
        for (int t = 0; t < 256; t++) {
          if (automaton->adjacency_matrix[edge].transitions[t]) {
            printf(" %s->%d", print_char(t), node1);
          }
        }
      }
    }
    printf("\n");
  }
}

automaton_t create_automaton(int node_count) {
  automaton_t result = {.adjacency_matrix =
                            calloc(node_count * node_count, sizeof(edge_t)),
                        .nodes = calloc(node_count, sizeof(node_t)),
                        .max_node_count = node_count,
                        .next_node_index = 0,
                        .start_index = 0};
  return result;
}

int create_node(automaton_t *automaton) { return automaton->next_node_index++; }

void connect_nodes(automaton_t *automaton, int node0, int node1,
                   unsigned char terminal, bool_t is_epsilon) {
  int transition = terminal;
  if (is_epsilon) {
    transition = EPSILON_EDGE;
  }
  automaton->adjacency_matrix[edge_idx(automaton, node0, node1)]
      .transitions[transition] = 1;
}

automaton_t determinize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}

automaton_t minimize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}
