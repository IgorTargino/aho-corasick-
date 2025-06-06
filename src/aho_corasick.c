#include "aho_corasick.h"
#include <stdio.h>
#include <string.h>

static const uint8_t ROOT_VERTEX = 0;

static void init_vertex(ac_vertex_t *vertex) {
    memset(vertex, 0, sizeof(ac_vertex_t));
    vertex->link = INVALID_VERTEX_U8;
    vertex->output = 0;
    vertex->num_patterns = 0;
    vertex->num_transitions = 0;
    
    for (int i = 0; i < AC_MAX_PATTERNS_PER_VERTEX; ++i) {
        vertex->pattern_indices[i] = INVALID_VERTEX_U8;
    }
}

static uint8_t allocate_vertex(ac_automaton_t *ac) {
    if (ac->current_vertex_count >= AC_MAX_VERTICES) {
        DEBUG_PRINTF("Error: Maximum vertices (%d) reached\n", AC_MAX_VERTICES);
        return INVALID_VERTEX_U8;
    }
    
    uint8_t new_vertex = ac->current_vertex_count++;
    init_vertex(&ac->vertices[new_vertex]);
    return new_vertex;
}

static inline bool is_valid_vertex(const ac_automaton_t *ac, uint8_t vertex_idx) {
    return vertex_idx != INVALID_VERTEX_U8 && vertex_idx < ac->current_vertex_count;
}

// Encontra transição para um caractere específico
static uint8_t find_transition(const ac_vertex_t *vertex, uint8_t character) {
    for (int i = 0; i < vertex->num_transitions; i++) {
        if (vertex->transitions[i].character == character) {
            return vertex->transitions[i].next_vertex;
        }
    }
    return INVALID_VERTEX_U8;
}

// Adiciona uma nova transição
static bool add_transition(ac_vertex_t *vertex, uint8_t character, uint8_t next_vertex) {
    if (vertex->num_transitions >= AC_MAX_TRANSITIONS_PER_VERTEX) {
        DEBUG_PRINTF("Warning: Maximum transitions per vertex (%d) reached\n", 
                    AC_MAX_TRANSITIONS_PER_VERTEX);
        return false;
    }
    
    // Verifica se já existe
    if (find_transition(vertex, character) != INVALID_VERTEX_U8) {
        return false; // Transição já existe
    }
    
    vertex->transitions[vertex->num_transitions].character = character;
    vertex->transitions[vertex->num_transitions].next_vertex = next_vertex;
    vertex->num_transitions++;
    return true;
}

static bool add_pattern_to_vertex(ac_vertex_t *vertex, uint8_t pattern_idx) {
    if (vertex->num_patterns >= AC_MAX_PATTERNS_PER_VERTEX) {
        DEBUG_PRINTF("Warning: Maximum patterns per vertex (%d) reached\n", AC_MAX_PATTERNS_PER_VERTEX);
        return false;
    }
    
    vertex->pattern_indices[vertex->num_patterns++] = pattern_idx;
    vertex->output = 1;
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

static uint8_t build_trie_path(ac_automaton_t *ac, const char* pattern, uint8_t pattern_idx) {
    uint8_t current = ROOT_VERTEX;
    
    for (int i = 0; pattern[i] != '\0'; ++i) {
        int char_idx = char_to_index(pattern[i]);
        if (char_idx == INVALID_VERTEX) {
            DEBUG_PRINTF("Warning: Invalid character '%c' in pattern, skipping\n", pattern[i]);
            continue;
        }
        
        uint8_t next_vertex = find_transition(&ac->vertices[current], (uint8_t)char_idx);
        
        if (next_vertex == INVALID_VERTEX_U8) {
            next_vertex = allocate_vertex(ac);
            if (next_vertex == INVALID_VERTEX_U8) {
                return INVALID_VERTEX_U8; // Allocation failed
            }
            
            if (!add_transition(&ac->vertices[current], (uint8_t)char_idx, next_vertex)) {
                return INVALID_VERTEX_U8; // Failed to add transition
            }
        }
        
        current = next_vertex;
    }
    
    add_pattern_to_vertex(&ac->vertices[current], pattern_idx);
    return current;
}

static uint8_t compute_go_iterative(ac_automaton_t *ac, uint8_t state, uint8_t character) {
    uint8_t current_state = state;
    
    while (current_state != INVALID_VERTEX_U8) {
        uint8_t direct = find_transition(&ac->vertices[current_state], character);
        if (direct != INVALID_VERTEX_U8) {
            return direct;
        }
        
        if (current_state == ROOT_VERTEX) {
            return ROOT_VERTEX;
        }
        
        current_state = ac->vertices[current_state].link;
    }
    
    return ROOT_VERTEX; // Fallback
}


static void initialize_root_transitions(ac_automaton_t *ac) {
    ac_vertex_t *root = &ac->vertices[ROOT_VERTEX];
    
    for (int i = 0; i < root->num_transitions; i++) {
        uint8_t child = root->transitions[i].next_vertex;
        ac->vertices[child].link = ROOT_VERTEX;
        aho_queue_enqueue(&ac->queue, child);
    }
}

static void compute_failure_links_optimized(ac_automaton_t *ac) {
    while (!aho_queue_is_empty(&ac->queue)) {
        uint8_t current = aho_queue_dequeue(&ac->queue);
        
        for (uint8_t i = 0; i < ac->vertices[current].num_transitions; i++) {
            uint8_t child = ac->vertices[current].transitions[i].next_vertex;
            ac->vertices[child].link = compute_go_iterative(ac, 
                ac->vertices[current].link, 
                ac->vertices[current].transitions[i].character);
            
            aho_queue_enqueue(&ac->queue, child);
        }
    }
}

static void report_matches_at_state(const ac_automaton_t *ac, uint8_t state, int text_position) {
    uint8_t current_state = state;
    
    while (current_state != INVALID_VERTEX_U8) {
        if (ac->vertices[current_state].output) {
            uint8_t num_patterns = ac->vertices[current_state].num_patterns;
            for (uint8_t i = 0; i < num_patterns; ++i) {
                uint8_t pattern_idx = ac->vertices[current_state].pattern_indices[i];
                if (pattern_idx < ac->num_total_patterns) {
                    ac_set_match_callback(ac->patterns[pattern_idx], text_position);
                }
            }
        }
        
        if (current_state == ROOT_VERTEX) break;
        current_state = ac->vertices[current_state].link;
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
    
    if (build_trie_path(ac, pattern, ac->num_total_patterns) == INVALID_VERTEX_U8) {
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
    
    aho_queue_init(&ac->queue);
    
    initialize_root_transitions(ac);
    compute_failure_links_optimized(ac);
    
    DEBUG_PRINTF("Automaton built with %d vertices and %d patterns\n", 
                ac->current_vertex_count, ac->num_total_patterns);
}

void ac_search(ac_automaton_t *ac, const char* text) {
    if (!ac || !text || ac->num_total_patterns == 0) {
        DEBUG_PRINTF("Warning: Invalid search parameters or empty automaton\n");
        return;
    }
    
    uint8_t current_state = ROOT_VERTEX;
    
    for (int i = 0; text[i] != '\0'; ++i) {
        int char_idx = char_to_index(text[i]);
        
        if (char_idx == INVALID_VERTEX) {
            current_state = ROOT_VERTEX;
            continue;
        }
        
        uint8_t next_state = compute_go_iterative(ac, current_state, (uint8_t)char_idx);
        current_state = next_state;
        report_matches_at_state(ac, current_state, i);
    }
}