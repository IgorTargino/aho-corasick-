#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include <stddef.h> 

#define AC_K_ALPHABET_SIZE 26
#define AC_MAX_VERTICES 40 // 40×213=8520 bytes (aproximadamente 8.3KB) 
typedef struct ac_vertex_s {
    int next[AC_K_ALPHABET_SIZE]; // Transições diretas da Trie para filhos
    int go[AC_K_ALPHABET_SIZE];   // Transições do autômato (calculadas)
    int link;                     // Link de falha (suffix link)
    bool output;                  // true se um padrão termina neste nó
} ac_vertex_t;

void ac_initialize_automaton(void);

void ac_add_pattern(const char* pattern);

void ac_build_automaton(void);

void ac_search(const char* text);

#endif 