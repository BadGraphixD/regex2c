// TODO: check if we want this datastructure for the automaton
//  alternatives:
//  - edge list (while nodes are in array)
//  - adjacency matrix
//  Nodes and edges can be evaluated by traversing the ast once before automaton
//  construction.

/**
 * We need to:
 *
 * 1. Convert the regex ast into the NFA (by inserting a ludicrous amount of
 * epsilon-transitions)
 * 2. Convert the NFA into the DFA
 * 3. Minimize the DFA
 * 4. Convert the DFA into c code
 */

struct edge_list;

typedef struct node {
  struct edge_list *outgoing;
  int is_end;
} node_t;

typedef struct edge {
  int target;
  unsigned char terminal;
  unsigned char flags;
} edge_t;

typedef struct edge_list {
  struct edge_list *next;
  edge_t edge;
} edge_list_t;

typedef struct automaton {
  node_t *nodes;
  int max_node_count;
  int next_node_index;
  int start_index;
} automaton_t;

/**
 * Prints the given {@code automaton}.
 */
void print_automaton(automaton_t *automaton);

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
void connect_nodes(automaton_t *automaton, int node0, int node1, char terminal,
                   char is_epsilon, char is_wildcard);

/**
 * Returns whether or not an edge is an epsilon-transition.
 */
int edge_is_epsilon(edge_t *edge);

/**
 * Returns whether or not an edge is an wildcard-transition.
 */
int edge_is_wildcard(edge_t *edge);

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
