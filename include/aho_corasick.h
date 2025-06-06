#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include <stdint.h>
#include "aho_queue.h"
#include "aho_config.h"

static const int INVALID_VERTEX = -1;

typedef struct ac_vertex ac_vertex_t;
typedef struct ac_automaton ac_automaton_t;

// Estrutura compacta para transições - apenas armazena transições existentes
typedef struct {
    uint8_t character;    // 1 byte - caractere ('a'-'z' = 0-25)
    uint8_t next_vertex;  // 1 byte - índice do próximo vértice
} ac_transition_t;

struct ac_vertex {
    // Transições diretas (trie) - apenas as que existem
    ac_transition_t transitions[AC_MAX_TRANSITIONS_PER_VERTEX];
    uint8_t num_transitions;  // 1 byte - número de transições
    
    uint8_t link;             // 1 byte - link de falha  
    uint8_t output : 1;       // 1 bit - flag de saída
    uint8_t num_patterns : 7; // 7 bits - número de padrões (max 127)
    
    // Índices dos padrões que terminam neste vértice
    uint8_t pattern_indices[AC_MAX_PATTERNS_PER_VERTEX];
};

struct ac_automaton {
    ac_vertex_t vertices[AC_MAX_VERTICES];
    uint8_t current_vertex_count;
    const char* patterns[AC_MAX_PATTERNS];
    uint8_t num_total_patterns;
    aho_queue_t queue;
};

// --- Automaton Operations ---
/**
 * Initialize the Aho-Corasick automaton
 * @param ac Pointer to automaton structure
 */
void ac_initialize_automaton(ac_automaton_t *ac);

/**
 * Add a pattern to the automaton
 * Must be called before ac_build_automaton()
 * @param ac Pointer to automaton structure
 * @param pattern Null-terminated string pattern (only A-Z, a-z allowed)
 * @return true if pattern added successfully, false otherwise
 */
bool ac_add_pattern(ac_automaton_t *ac, const char* pattern);

/**
 * Build the automaton after all patterns have been added
 * Computes failure links and go transitions
 * @param ac Pointer to automaton structure
 */
void ac_build_automaton(ac_automaton_t *ac);

/**
 * Search for patterns in text
 * @param ac Pointer to automaton structure
 * @param text Text to search in
 */
void ac_search(ac_automaton_t *ac, const char* text);

extern void ac_set_match_callback(const char* pattern, int position);

/**
 * Get current number of vertices in the automaton
 * @param ac Pointer to automaton structure
 * @return Number of allocated vertices
 */
static inline int ac_get_vertex_count(const ac_automaton_t *ac) {
    return ac ? ac->current_vertex_count : 0;
}

/**
 * Get total number of patterns in the automaton
 * @param ac Pointer to automaton structure
 * @return Number of patterns added
 */
static inline int ac_get_pattern_count(const ac_automaton_t *ac) {
    return ac ? ac->num_total_patterns : 0;
}

/**
 * Check if automaton has been built (ready for searching)
 * @param ac Pointer to automaton structure
 * @return true if automaton is built and ready for search
 */
static inline bool ac_is_built(const ac_automaton_t *ac) {
    return ac && ac->current_vertex_count > 0 && ac->num_total_patterns > 0;
}

static inline int char_to_index(char c) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 'A' && uc <= 'Z') {
        return uc - 'A';
    }
    if (uc >= 'a' && uc <= 'z') {
        return uc - 'a';
    }
    return INVALID_VERTEX;
}

#endif