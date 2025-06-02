#ifndef AHO_TRIE_H
#define AHO_TRIE_H

#include "aho_corasick.h"

void aho_trie_init(ac_vertex_t* v);
int aho_trie_add_pattern(ac_vertex_t* trie, int* nodes_count, const char* pattern);

#endif 