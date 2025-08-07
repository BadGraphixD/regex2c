#include "automaton.h"
#include "common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((always_inline)) inline int edge_idx(automaton_t *automaton,
                                                   int node0, int node1) {
  return node1 * automaton->max_node_count + node0;
}

void print_automaton(automaton_t *automaton) {
  printf("Automaton (max nodes = %d, node count = %d)\n",
         automaton->max_node_count, automaton->next_node_index);

  for (int node0 = 0; node0 < automaton->max_node_count; node0++) {
    if (automaton->nodes[node0].end_tag == -1) {
      printf("%c     #%d", automaton->start_index == node0 ? '>' : ' ', node0);
    } else {
      printf("%c*%3d #%d", automaton->start_index == node0 ? '>' : ' ',
             automaton->nodes[node0].end_tag, node0);
    }

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
                        .nodes = malloc(node_count * sizeof(node_t)),
                        .max_node_count = node_count,
                        .next_node_index = 0,
                        .start_index = 0};
  memset(result.nodes, 0xff, node_count * sizeof(node_t));
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
  dfa_edge_list_t *outgoing;
  int index;
  int end_tag;
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
  state.end_tag = -1;
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

__attribute__((always_inline)) inline void
set_dfa_state_node(dfa_state_t *state, int node) {
  state->nodes[node] = 1;
}

__attribute__((always_inline)) inline bool_t
get_dfa_state_node(dfa_state_t *state, int node) {
  return state->nodes[node];
}

__attribute__((always_inline)) inline int choose_end_tag(int old, int new) {
  return new != -1 && (old == -1 || old > new) ? new : old;
  /* if (new != -1 && (old == -1 || old > new)) { */
  /*   return new; */
  /* } */
  /* return old; */
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
          move.end_tag =
              choose_end_tag(move.end_tag, automaton->nodes[node1].end_tag);
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
            closure.end_tag = choose_end_tag(closure.end_tag,
                                             automaton->nodes[node1].end_tag);
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
    automaton.nodes[list->state.index].end_tag = list->state.end_tag;
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

/**
 * Returns whether or not the transitions from the given {@code node0} and
 * {@code node1} result in the same partition for every terminal.
 * (See Moore's Algorithm)
 */
bool_t nodes_equivalent(bool_t *stm, int node0, int node1, int *partition) {
  for (int t = 0; t < 256; t++) {
    int dest0 = stm[node0 * 256 + t];
    int dest1 = stm[node1 * 256 + t];
    if (partition[dest0] != partition[dest1]) {
      if (PRINT_DEBUG >= 2) {
        printf("Nodes %d and %d different because targets %d and %d are in "
               "different partitions\n",
               node0, node1, dest0, dest1);
      }
      return 0;
    }
  }
  return 1;
}

bool_t *create_state_transition_matrix(automaton_t *automaton) {
  int N = automaton->max_node_count;
  size_t stm_size = N * 256 * sizeof(bool_t);
  bool_t *stm = malloc(stm_size);
  memset(stm, 0xff, stm_size);
  for (int start = 0; start < N; start++) {
    for (int end = 0; end < N; end++) {
      for (int t = 0; t < 256; t++) {
        if (automaton->adjacency_matrix[edge_idx(automaton, start, end)]
                .transitions[t]) {
          stm[start * 256 + t] = end;
        }
      }
    }
  }
  return stm;
}

bool_t partitions_equivalent(int *p0, int *p1, int N) {
  for (int i = 0; i < N; i++) {
    if (p0[i] != p1[i]) {
      return 0;
    }
  }
  return 1;
}

void print_partition(int *p, int N) {
  for (int i = 0; i < N; i++) {
    printf(" %d", p[i]);
  }
  printf("\n");
}

automaton_t create_automaton_from_partition(automaton_t *automaton,
                                            int *partition, int node_count) {
  automaton_t result = create_automaton(node_count);
  int old_N = automaton->max_node_count;
  for (int i = 0; i < old_N; i++) {
    for (int j = 0; j < old_N; j++) {
      for (int t = 0; t < 256; t++) {
        if (automaton->adjacency_matrix[edge_idx(automaton, i, j)]
                .transitions[t]) {
          connect_nodes(&result, partition[i], partition[j], t, 0);
        }
      }
    }
    int end_tag = automaton->nodes[i].end_tag;
    if (end_tag != -1) {
      result.nodes[partition[i]].end_tag = end_tag;
    }
  }
  return result;
}

automaton_t minimize(automaton_t *automaton) {
  int N = automaton->max_node_count;
  size_t partition_size = N * sizeof(int);
  int *partition0 = malloc(partition_size);
  int *partition1 = malloc(partition_size);
  // set all values to -1
  memset(partition0, 0xff, partition_size);
  memset(partition1, 0xff, partition_size);
  bool_t *stm = create_state_transition_matrix(automaton);

  // initial node partition (end states vs normal state)
  for (int i = 0; i < N; i++) {
    partition0[i] = automaton->nodes[i].end_tag;
  }

  if (PRINT_DEBUG >= 2) {
    printf("Initial partition:");
    print_partition(partition0, N);
  }

  while (1) {
    int next_partition_idx = 0;

    int i = 0;
    while (i < N) {
      if (PRINT_DEBUG >= 2) {
        printf("\n\n\nNew round, starting from %d:\n", i);
        printf("This is the old partition:");
        print_partition(partition0, N);
      }
      partition1[i] = next_partition_idx;
      int i_next = N;
      for (int j = i + 1; j < N; j++) {
        if (PRINT_DEBUG >= 2) {
          printf("Checking nodes i=%d and j=%d:   ", i, j);
        }
        if (partition1[j] >= 0) {
          if (PRINT_DEBUG >= 2) {
            printf("Node j=%d already in p=%d\n", j, partition1[j]);
          }
          // that node is already partitioned
          continue;
        }
        if (partition0[i] == partition0[j] &&
            nodes_equivalent(stm, i, j, partition0)) {
          // nodes are equivalent -> put into same partition
          if (PRINT_DEBUG >= 2) {
            printf("Nodes equivalent\n");
          }
          partition1[j] = next_partition_idx;
        } else if (i_next == N) {
          // nodes are not equivalent -> create new partition and
          // start at this node
          i_next = j;
          if (PRINT_DEBUG >= 2) {
            printf("Nodes not equivalent, let's start at %d next round :)\n",
                   i_next);
          }
        } else if (PRINT_DEBUG >= 2) {
          printf("Nodes not equivalent\n");
        }
      }
      i = i_next;
      next_partition_idx++;

      if (PRINT_DEBUG >= 2) {
        printf("This is the new partition:");
        print_partition(partition1, N);
      }
    }

    if (PRINT_DEBUG >= 2) {
      printf("This is the completed partition:");
      print_partition(partition1, N);
    }

    if (partitions_equivalent(partition0, partition1, N)) {
      automaton_t result = create_automaton_from_partition(
          automaton, partition1, next_partition_idx);

      free(partition0);
      free(partition1);
      free(stm);

      return result;
    }

    // swap partitions and reset one
    int *tmp = partition0;
    partition0 = partition1;
    partition1 = tmp;
    memset(partition1, 0xff, partition_size);
  }
}

void delete_automaton(automaton_t automaton) {
  free(automaton.adjacency_matrix);
  free(automaton.nodes);
}
