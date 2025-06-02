#include "aho_trie.h"
#include <stdio.h>
#include <string.h>

void aho_trie_init(ac_vertex_t* v) {
    if (v == NULL) return;
    v->link = 0;
    v->output = false;
    for (int i = 0; i < AC_K_ALPHABET_SIZE; ++i) {
        v->next[i] = -1;
        v->go[i] = -1;
    }
}

int aho_trie_add_pattern(ac_vertex_t* trie, int* nodes_count, const char* pattern, int pattern_idx) {
    int current_node_idx = 0;
    for (int i = 0; pattern[i] != '\0'; ++i) {
        char ch = pattern[i];
        if (ch < 'a' || ch > 'z') continue;
        int char_idx = ch - 'a';
        if (trie[current_node_idx].next[char_idx] == -1) {
            if (*nodes_count >= AC_MAX_VERTICES) {
                return -1;
            }
            trie[current_node_idx].next[char_idx] = (*nodes_count);
            aho_trie_init(&trie[*nodes_count]);
            (*nodes_count)++;
        }
        current_node_idx = trie[current_node_idx].next[char_idx];
    }
    trie[current_node_idx].output = true;
    if (trie[current_node_idx].num_patterns < AC_MAX_PATTERNS) {
        trie[current_node_idx].pattern_indices[trie[current_node_idx].num_patterns++] = pattern_idx;
    }
    return 0;
}