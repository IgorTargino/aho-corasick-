#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include <stdint.h>
#include "aho_queue.h"
#include "aho_config.h"

typedef void (*ac_match_callback_t)(const char* pattern, int position);
typedef struct {
    uint8_t character;    // 1 byte - caractere ('a'-'z' = 0-25)
    uint8_t next_vertex;  // 1 byte - índice do próximo vértice
} ac_transition_t;

typedef struct ac_vertex {
    ac_transition_t transitions[AC_MAX_TRANSITIONS_PER_VERTEX];
    uint8_t num_transitions;  // 1 byte - número de transições
    uint8_t link;             // 1 byte - link de falha  
    uint8_t is_output : 1;       // 1 bit - flag de saída
    uint8_t num_patterns : 7; // 7 bits - número de padrões (max 127)												
    uint8_t pattern_indices[AC_MAX_PATTERNS_PER_VERTEX]; // Índices dos padrões que terminam neste vértice 
} ac_vertex_t;

typedef struct ac_automaton {
    ac_vertex_t vertices[AC_MAX_VERTICES];
    uint8_t vertex_count;
    const char* patterns[AC_MAX_PATTERNS];
    uint8_t pattern_count;
    aho_queue_t queue;
    ac_match_callback_t match_callback;
} ac_automaton_t;

void ac_init(ac_automaton_t *ac, ac_match_callback_t callback);
bool ac_add_pattern(ac_automaton_t *ac, const char* pattern);
void ac_build(ac_automaton_t *ac);
void ac_search(ac_automaton_t *ac, const char* text);

#endif // AHO_CORASICK_H