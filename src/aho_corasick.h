#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>

#define AC_MAX_VERTICES 40
#define AC_K_ALPHABET_SIZE 26

typedef struct {
    int next[AC_K_ALPHABET_SIZE];
    int go[AC_K_ALPHABET_SIZE];
    int link;
    bool output;
} ac_vertex_t;

void ac_initialize_automaton(void);
void ac_add_pattern(const char* pattern);
void ac_build_automaton(void);
void ac_search(const char* text);

#endif