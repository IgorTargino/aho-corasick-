#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include "aho_queue.h"
#include "aho_config.h"


typedef struct ac_vertex ac_vertex_t;
typedef struct ac_automaton ac_automaton_t;

// --- Automaton Vertex Structure ---
struct ac_vertex {
    // Trie structure
    int next[AC_K_ALPHABET_SIZE];           // Direct transitions (trie edges)
    int go[AC_K_ALPHABET_SIZE];             // Computed transitions (automaton edges)
    int link;                               // Failure link (suffix link)
    
    // Pattern information
    bool output;                            // True if this vertex represents end of pattern(s)
    int pattern_indices[AC_MAX_PATTERNS_PER_VERTEX];  // Indices of patterns ending here
    int num_patterns_at_vertex;             // Number of patterns ending at this vertex
};

// --- Main Automaton Structure ---
struct ac_automaton {
    ac_vertex_t vertices[AC_MAX_VERTICES];
    int current_vertex_count;
    const char* patterns[AC_MAX_PATTERNS];
    int num_total_patterns;
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
 * Search for all patterns in the given text
 * Calls ac_set_match_callback for each match found
 * @param ac Pointer to built automaton structure
 * @param text Null-terminated text to search in
 */
void ac_search(ac_automaton_t *ac, const char* text);

// --- Match Callback ---
/**
 * Callback function called when a pattern match is found
 * This function must be implemented by the user of the library
 * @param pattern The matched pattern string
 * @param position Position in text where pattern ends (0-based)
 */
extern void ac_set_match_callback(const char* pattern, int position);

// --- Utility Functions ---
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

#endif // AHO_CORASICK_H