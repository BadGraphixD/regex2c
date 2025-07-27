#include "automaton.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void print_automaton(automaton_t *automaton) {
  printf("Automaton (max nodes = %d, node count = %d)\n",
         automaton->max_node_count, automaton->next_node_index);
  for (int i = 0; i < automaton->max_node_count; i++) {
    printf("%c%c #%d", automaton->start_index == i ? '>' : ' ',
           automaton->nodes[i].is_end ? '*' : ' ', i);
    edge_list_t *list = automaton->nodes[i].outgoing;
    while (list != NULL) {
      if (edge_is_epsilon(&list->edge)) {
        printf(" ε->%d", list->edge.target);
      } else if (edge_is_wildcard(&list->edge)) {
        printf(" any->%d", list->edge.target);
      } else {
        printf(" %s->%d", print_char(list->edge.terminal), list->edge.target);
      }
      list = list->next;
    }
    printf("\n");
  }
}

automaton_t create_automaton(int node_count) {
  automaton_t result = {.nodes = calloc(node_count, sizeof(node_t)),
                        .max_node_count = node_count,
                        .next_node_index = 0,
                        .start_index = 0};
  return result;
}

int create_node(automaton_t *automaton) { return automaton->next_node_index++; }

void connect_nodes(automaton_t *automaton, int node0, int node1, char terminal,
                   char is_epsilon, char is_wildcard) {
  edge_list_t *list = malloc(sizeof(edge_list_t));
  list->next = automaton->nodes[node0].outgoing;
  list->edge.flags = 0;
  if (is_epsilon) {
    list->edge.flags |= 0x1;
  }
  if (is_wildcard) {
    list->edge.flags |= 0x2;
  }
  list->edge.terminal = terminal;
  list->edge.target = node1;
  automaton->nodes[node0].outgoing = list;
}

int edge_is_epsilon(edge_t *edge) { return edge->flags & 0x1; }

int edge_is_wildcard(edge_t *edge) { return edge->flags & 0x2; }

automaton_t determinize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}

automaton_t minimize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}
