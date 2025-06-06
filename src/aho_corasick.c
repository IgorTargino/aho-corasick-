#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>

static const int ROOT_VERTEX = 0;

static void init_vertex(ac_vertex_t *vertex) {
    memset(vertex, 0, sizeof(ac_vertex_t));
    
    for (int i = 0; i < AC_K_ALPHABET_SIZE; ++i) {
        vertex->next[i] = INVALID_VERTEX;
        vertex->go[i] = INVALID_VERTEX;
    }
    vertex->link = INVALID_VERTEX;
    
    for (int i = 0; i < AC_MAX_PATTERNS_PER_VERTEX; ++i) {
        vertex->pattern_indices[i] = INVALID_VERTEX;
    }
}

static int allocate_vertex(ac_automaton_t *ac) {
    if (ac->current_vertex_count >= AC_MAX_VERTICES) {
        DEBUG_PRINTF("Error: Maximum vertices (%d) reached\n", AC_MAX_VERTICES);
        return INVALID_VERTEX;
    }
    
    int new_vertex = ac->current_vertex_count++;
    init_vertex(&ac->vertices[new_vertex]);
    return new_vertex;
}

static inline bool is_valid_vertex(const ac_automaton_t *ac, int vertex_idx) {
    return vertex_idx >= 0 && vertex_idx < ac->current_vertex_count;
}

static bool add_pattern_to_vertex(ac_vertex_t *vertex, int pattern_idx) {
    if (vertex->num_patterns_at_vertex >= AC_MAX_PATTERNS_PER_VERTEX) {
        DEBUG_PRINTF("Warning: Maximum patterns per vertex (%d) reached\n", AC_MAX_PATTERNS_PER_VERTEX);
        return false;
    }
    
    vertex->pattern_indices[vertex->num_patterns_at_vertex++] = pattern_idx;
    vertex->output = true;
    return true;
}

static int calculate_pattern_depth(const char* pattern) {
    int depth = 0;
    for (int i = 0; pattern[i] != '\0'; ++i) {
        if (char_to_index(pattern[i]) != INVALID_VERTEX) {
            depth++;
        }
    }
    return depth;
}

static int build_trie_path(ac_automaton_t *ac, const char* pattern, int pattern_idx) {
    int current = ROOT_VERTEX;
    
    for (int i = 0; pattern[i] != '\0'; ++i) {
        int char_idx = char_to_index(pattern[i]);
        if (char_idx == INVALID_VERTEX) {
            DEBUG_PRINTF("Warning: Invalid character '%c' in pattern, skipping\n", pattern[i]);
            continue;
        }
        
        if (ac->vertices[current].next[char_idx] == INVALID_VERTEX) {
            int new_vertex = allocate_vertex(ac);
            if (new_vertex == INVALID_VERTEX) {
                return INVALID_VERTEX; // Allocation failed
            }
            ac->vertices[current].next[char_idx] = new_vertex;
        }
        
        current = ac->vertices[current].next[char_idx];
    }
    
    add_pattern_to_vertex(&ac->vertices[current], pattern_idx);
    return current;
}

static void initialize_root_transitions(ac_automaton_t *ac) {
    ac_vertex_t *root = &ac->vertices[ROOT_VERTEX];
    
    for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
        if (root->next[c] != INVALID_VERTEX) {
            root->go[c] = root->next[c];
            ac->vertices[root->next[c]].link = ROOT_VERTEX;
            aho_queue_enqueue(&ac->queue, root->next[c]);
        } else {
            root->go[c] = ROOT_VERTEX;
        }
    }
}

static void compute_failure_links(ac_automaton_t *ac) {
    while (!aho_queue_is_empty(&ac->queue)) {
        int current = aho_queue_dequeue(&ac->queue);
        ac_vertex_t *current_vertex = &ac->vertices[current];
        
        for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
            int child = current_vertex->next[c];
            
            if (child != INVALID_VERTEX) {
                current_vertex->go[c] = child;
                
                int failure_state = current_vertex->link;
                ac->vertices[child].link = ac->vertices[failure_state].go[c];
                
                if (ac->vertices[ac->vertices[child].link].output) {
                    ac->vertices[child].output = true;
                }
                
                aho_queue_enqueue(&ac->queue, child);
            } else {
                int failure_state = current_vertex->link;
                current_vertex->go[c] = ac->vertices[failure_state].go[c];
            }
        }
    }
}

static void report_matches_at_state(const ac_automaton_t *ac, int state, int text_position) {
    while (state != INVALID_VERTEX) {
        const ac_vertex_t *vertex = &ac->vertices[state];
        
        if (vertex->output) {
            for (int i = 0; i < vertex->num_patterns_at_vertex; ++i) {
                int pattern_idx = vertex->pattern_indices[i];
                if (pattern_idx >= 0 && pattern_idx < ac->num_total_patterns) {
                    ac_set_match_callback(ac->patterns[pattern_idx], text_position);
                }
            }
        }
        
        if (state == ROOT_VERTEX) break;
        state = vertex->link;
    }
}

void ac_initialize_automaton(ac_automaton_t *ac) {
    if (!ac) return;
    
    ac->current_vertex_count = 0;
    ac->num_total_patterns = 0;
    aho_queue_init(&ac->queue);
    
    if (allocate_vertex(ac) != ROOT_VERTEX) {
        DEBUG_PRINTF("Error: Failed to allocate root vertex\n");
        return;
    }
    ac->vertices[ROOT_VERTEX].link = ROOT_VERTEX;
}

bool ac_add_pattern(ac_automaton_t *ac, const char* pattern) {
    if (!ac || !pattern || *pattern == '\0') {
        DEBUG_PRINTF("Error: Invalid automaton or empty pattern\n");
        return false;
    }
    
    if (ac->num_total_patterns >= AC_MAX_PATTERNS) {
        DEBUG_PRINTF("Error: Maximum patterns (%d) reached\n", AC_MAX_PATTERNS);
        return false;
    }
    
    int required_depth = calculate_pattern_depth(pattern);
    if (ac->current_vertex_count + required_depth > AC_MAX_VERTICES) {
        DEBUG_PRINTF("Error: Not enough vertices for pattern. Required: %d, Available: %d\n",
                    required_depth, AC_MAX_VERTICES - ac->current_vertex_count);
        return false;
    }
    
    ac->patterns[ac->num_total_patterns] = pattern;
    
    if (build_trie_path(ac, pattern, ac->num_total_patterns) == INVALID_VERTEX) {
        return false;
    }
    
    ac->num_total_patterns++;
    DEBUG_PRINTF("Pattern \"%s\" added. Total patterns: %d\n", pattern, ac->num_total_patterns);
    return true;
}

void ac_build_automaton(ac_automaton_t *ac) {
    if (!ac || ac->current_vertex_count <= 1) {
        DEBUG_PRINTF("Warning: Building empty or uninitialized automaton\n");
        if (ac && ac->current_vertex_count == 1) {
            initialize_root_transitions(ac);
        }
        return;
    }
    
    initialize_root_transitions(ac);
    compute_failure_links(ac);
    
    DEBUG_PRINTF("Automaton built with %d vertices and %d patterns\n", 
                ac->current_vertex_count, ac->num_total_patterns);
}

void ac_search(ac_automaton_t *ac, const char* text) {
    if (!ac || !text || ac->num_total_patterns == 0) {
        DEBUG_PRINTF("Warning: Invalid search parameters or empty automaton\n");
        return;
    }
    
    int current_state = ROOT_VERTEX;
    
    for (int i = 0; text[i] != '\0'; ++i) {
        int char_idx = char_to_index(text[i]);
        
        if (char_idx == INVALID_VERTEX) {
            current_state = ROOT_VERTEX; // Reset on invalid character
            continue;
        }
        
        int next_state = ac->vertices[current_state].go[char_idx];
        if (next_state == INVALID_VERTEX) {
            DEBUG_PRINTF("Critical error: Uncomputed go transition from state %d, char %d\n", 
                        current_state, char_idx);
            current_state = ROOT_VERTEX;
            continue;
        }
        
        current_state = next_state;
        report_matches_at_state(ac, current_state, i);
    }
}