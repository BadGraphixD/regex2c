#include "automaton.h"
#include "common.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct dfa_edge {
  int target;
  bool_t transitions[256];
} dfa_edge_t;

typedef struct dfa_edge_list {
  struct dfa_edge_list *next;
  dfa_edge_t edge;
} dfa_edge_list_t;

typedef struct dfa_state {
  bool_t *nodes;
  int index;
  dfa_edge_list_t *outgoing;
  bool_t is_end;
} dfa_state_t;

typedef struct dfa_state_list {
  struct dfa_state_list *next;
  dfa_state_t state;
} dfa_state_list_t;

dfa_state_t create_dfa_state(automaton_t *automaton, int index) {
  dfa_state_t state;
  state.nodes = calloc(automaton->max_node_count, sizeof(bool_t));
  state.index = index;
  state.outgoing = NULL;
  state.is_end = 0;
  return state;
}

void delete_dfa_state(dfa_state_t state) {
  free(state.nodes);
  dfa_edge_list_t *list = state.outgoing;
  while (list != NULL) {
    dfa_edge_list_t *next = list->next;
    free(list);
    list = next;
  }
}

void delete_dfa_state_list(dfa_state_list_t *list) {
  while (list != NULL) {
    dfa_state_list_t *next = list->next;
    delete_dfa_state(list->state);
    free(list);
    list = next;
  }
}

void set_dfa_state_node(dfa_state_t *state, int node) {
  state->nodes[node] = 1;
}

bool_t get_dfa_state_node(dfa_state_t *state, int node) {
  return state->nodes[node];
}

/**
 * Returns all nodes, which can be reached from any of the given {@code nodes}
 * via a transition of {@code terminal}.
 */
dfa_state_t move(automaton_t *automaton, dfa_state_t *state, int terminal,
                 int next_idx) {
  dfa_state_t move = create_dfa_state(automaton, next_idx);
  for (int node0 = 0; node0 < automaton->max_node_count; node0++) {
    if (get_dfa_state_node(state, node0)) {
      for (int node1 = 0; node1 < automaton->max_node_count; node1++) {
        int edge = edge_idx(automaton, node0, node1);
        if (automaton->adjacency_matrix[edge].transitions[terminal]) {
          set_dfa_state_node(&move, node1);
          if (automaton->nodes[node1].is_end) {
            move.is_end = 1;
          }
        }
      }
    }
  }
  return move;
}

/**
 * Returns all nodes, which can be reached from any of the given {@code nodes}
 * via an epsilon-transition. It also includes all nodes in {@code nodes}. This
 * is the epsilon-close of {@code nodes}.
 */
dfa_state_t make_epsclosure(automaton_t *automaton, dfa_state_t closure) {
  bool_t closure_changed;
  do {
    closure_changed = 0;
    for (int node0 = 0; node0 < automaton->max_node_count; node0++) {
      if (get_dfa_state_node(&closure, node0)) {
        for (int node1 = 0; node1 < automaton->max_node_count; node1++) {
          int edge = edge_idx(automaton, node0, node1);
          if (!get_dfa_state_node(&closure, node1) &&
              automaton->adjacency_matrix[edge].transitions[EPSILON_EDGE]) {
            set_dfa_state_node(&closure, node1);
            if (automaton->nodes[node1].is_end) {
              closure.is_end = 1;
            }
            closure_changed = 1;
          }
        }
      }
    }
  } while (closure_changed);
  return closure;
}

/**
 * Returns all nodes, which are start nodes.
 */
dfa_state_t initial_state(automaton_t *automaton) {
  dfa_state_t start = create_dfa_state(automaton, 0);
  set_dfa_state_node(&start, automaton->start_index);
  return start;
}

bool_t dfa_state_empty(automaton_t *automaton, dfa_state_t *state) {
  for (int node = 0; node < automaton->max_node_count; node++) {
    if (get_dfa_state_node(state, node)) {
      return 0;
    }
  }
  return 1;
}

bool_t dfa_states_equal(automaton_t *automaton, dfa_state_t *s0,
                        dfa_state_t *s1) {
  for (int node = 0; node < automaton->max_node_count; node++) {
    if (get_dfa_state_node(s0, node) != get_dfa_state_node(s1, node)) {
      return 0;
    }
  }
  return 1;
}

dfa_state_list_t *create_dfa_state_list(dfa_state_t state,
                                        dfa_state_list_t *next) {
  dfa_state_list_t *list = malloc(sizeof(dfa_state_list_t));
  list->state = state;
  list->next = next;
  return list;
}

/**
 * If the given {@code list} contains an equal state as {@code state}, then a
 * pointer to that equal state is returned. {@code NULL} otherwise.
 */
dfa_state_t *dfa_state_list_contains_state(automaton_t *automaton,
                                           dfa_state_list_t *list,
                                           dfa_state_t *state) {
  while (list != NULL) {
    if (dfa_states_equal(automaton, &list->state, state)) {
      return &list->state;
    }
    list = list->next;
  }
  return NULL;
}

void connect_dfa_states(dfa_state_t *start, dfa_state_t *end,
                        unsigned char terminal) {
  dfa_edge_list_t *list = start->outgoing;
  while (list != NULL) {
    if (list->edge.target == end->index) {
      list->edge.transitions[terminal] = 1;
      return;
    }
    list = list->next;
  }
  list = malloc(sizeof(dfa_edge_list_t));
  list->next = start->outgoing;
  list->edge.target = end->index;
  list->edge.transitions[terminal] = 1;
  start->outgoing = list;
}

automaton_t dfa_state_list_to_automaton(dfa_state_list_t *states) {
  dfa_state_list_t *list = states;
  int node_count = 0;
  while (list != NULL) {
    node_count++;
    list = list->next;
  }
  automaton_t automaton = create_automaton(node_count);
  automaton.start_index = 0;
  list = states;
  while (list != NULL) {
    dfa_edge_list_t *edge_list = list->state.outgoing;
    while (edge_list != NULL) {
      for (int t = 0; t < 256; t++) {
        if (edge_list->edge.transitions[t]) {
          connect_nodes(&automaton, list->state.index, edge_list->edge.target,
                        t, 0);
        }
      }
      edge_list = edge_list->next;
    }
    automaton.nodes[list->state.index].is_end = list->state.is_end;
    list = list->next;
  }
  delete_dfa_state_list(states);
  return automaton;
}

automaton_t determinize(automaton_t *automaton) {
  dfa_state_list_t *d_states = create_dfa_state_list(
      make_epsclosure(automaton, initial_state(automaton)), NULL);
  bool_t state_changed;
  int next_idx = 1;
  do {
    state_changed = 0;
    dfa_state_list_t *d_states_iter = d_states;
    while (d_states_iter != NULL) {
      for (int t = 0; t < 256; t++) {
        dfa_state_t new_state = make_epsclosure(
            automaton, move(automaton, &d_states_iter->state, t, next_idx));
        if (!dfa_state_empty(automaton, &new_state)) {
          dfa_state_t *existing_state =
              dfa_state_list_contains_state(automaton, d_states, &new_state);
          if (!existing_state) {
            d_states = create_dfa_state_list(new_state, d_states);
            existing_state = &d_states->state;
            state_changed = 1;
            next_idx++;
          } else {
            delete_dfa_state(new_state);
          }
          connect_dfa_states(&d_states_iter->state, existing_state, t);
        }
      }
      d_states_iter = d_states_iter->next;
    }
  } while (state_changed);
  return dfa_state_list_to_automaton(d_states);
}

automaton_t minimize(automaton_t *automaton) {
  // TODO: write this
  return *automaton;
}

void delete_automaton(automaton_t automaton) {
  free(automaton.adjacency_matrix);
  free(automaton.nodes);
}
