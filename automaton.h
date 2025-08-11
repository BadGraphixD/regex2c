/**
 * We need to:
 *
 * 1. Convert the regex ast into the NFA (by inserting a ludicrous amount of
 * epsilon-transitions)
 * 2. Convert the NFA into the DFA
 * 3. Minimize the DFA
 * 4. Convert the DFA into c code
 */

#pragma once

#include <stdio.h>

#include "common.h"

#define EPSILON_EDGE 256
#define MAX_EDGES 257

typedef struct edge {
  bool_t transitions[MAX_EDGES];
} edge_t;

typedef struct node {
  int end_tag;
} node_t;

typedef struct automaton {
  edge_t *adjacency_matrix;
  node_t *nodes;
  int max_node_count;
  int next_node_index;
  int start_index;
} automaton_t;

/**
 * Prints the given {@code automaton}.
 */
void print_automaton(automaton_t *automaton, FILE *fout);

/**
 * Creates an automaton which can hold up to {@code node_count} nodes.
 */
automaton_t create_automaton(int node_count);

/**
 * Creates a new node in the given {@code automaton}.
 */
int create_node(automaton_t *automaton);

/**
 * Creates a new connection between two nodes specified by their indices ({@code
 * node0} and {@code node1}). The connection is either an epsilon connection (if
 * {@code is_epsilon != 0}) or a normal connection with the {@code terminal}.
 */
void connect_nodes(automaton_t *automaton, int node0, int node1,
                   unsigned char terminal, bool_t is_epsilon);

/**
 * Creates a new automaton, which is equivalent to the given {@code automaton},
 * but is deterministic.
 */
automaton_t determinize(automaton_t *automaton);

/**
 * Creates a new automaton, which is equivalent to the given {@code automaton},
 * but is minimal. The given {@code automaton} must be deterministic.
 */
automaton_t minimize(automaton_t *automaton);

/**
 * Deletes a given {@code automaton} and frees all its related memory.
 */
void delete_automaton(automaton_t automaton);

/**
 * Creates a state transition matrix for the given {@code automaton}.
 */
bool_t *create_state_transition_matrix(automaton_t *automaton);
