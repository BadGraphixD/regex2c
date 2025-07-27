#include "automaton.h"

#include <stdio.h>
#include <stdlib.h>

void print_automaton(automaton_t *automaton) {
  printf("Automaton (max nodes = %d, node count = %d)\n",
         automaton->max_node_count, automaton->next_node_index);
  for (int i = 0; i < automaton->max_node_count; i++) {
    printf("  #%d", i);
    if (automaton->nodes[i].outgoing == NULL) {
      printf("no outgoing edges\n");
    } else {
      edge_list_t *list = automaton->nodes[i].outgoing;
      while (list != NULL) {
        if (list->edge.is_epsilon) {
          printf(" (ε: %d)", list->edge.target);
        } else {
          printf(" (%c: %d)", list->edge.terminal, list->edge.target);
        }
      }
    }
  }
}

automaton_t create_automaton(int node_count) {
  automaton_t result = {.nodes = calloc(node_count, sizeof(node_t)),
                        .max_node_count = node_count,
                        .next_node_index = 0};
  return result;
}

int create_node(automaton_t *automaton) { return automaton->next_node_index++; }

void connect_nodes(automaton_t *automaton, int node0, int node1, char terminal,
                   char is_epsilon) {
  edge_list_t *list = malloc(sizeof(edge_list_t));
  list->next = automaton->nodes[node0].outgoing;
  list->edge.is_epsilon = is_epsilon;
  list->edge.terminal = terminal;
  list->edge.target = node1;
  automaton->nodes[node0].outgoing = list;
}

automaton_t determinize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}

automaton_t minimize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}
