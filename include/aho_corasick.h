#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include "aho_config.h"
#include "aho_queue.h"

typedef struct {
    int next[AC_K_ALPHABET_SIZE];
    int go[AC_K_ALPHABET_SIZE];
    int link;
    bool output;
    int pattern_indices[AC_MAX_PATTERNS];
    int num_patterns;
} ac_vertex_t;

typedef struct {
    ac_vertex_t trie_automaton_nodes[AC_MAX_VERTICES];
    int current_nodes_count;
    const char* patterns[AC_MAX_PATTERNS];
    int num_patterns;
    aho_queue_t queue;
} ac_automaton_t;

extern void ac_on_match(const char* pattern, int pos, int pattern_index);

void ac_initialize_automaton(ac_automaton_t *ac);
void ac_add_pattern(ac_automaton_t *ac, const char* pattern);
void ac_build_automaton(ac_automaton_t *ac);
void ac_search(ac_automaton_t *ac, const char* text);

#endif