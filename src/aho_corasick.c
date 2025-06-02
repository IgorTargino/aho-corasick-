#include "aho_corasick.h"
#include "aho_queue.h"
#include "aho_trie.h"
#include <stdio.h>
#include <string.h>

void ac_initialize_automaton(ac_automaton_t *ac) {
    ac->current_nodes_count = 0;
    ac->num_patterns = 0;
    if (AC_MAX_VERTICES > 0) {
        aho_trie_init(&ac->trie_automaton_nodes[ac->current_nodes_count++]);
    } else {
        fprintf(stderr, "Erro Aho-Corasick: AC_MAX_VERTICES deve ser maior que 0.\n");
        return;
    }
    aho_queue_init(&ac->queue);
}

void ac_add_pattern(ac_automaton_t *ac, const char* pattern) {
    if (ac->current_nodes_count == 0) {
        fprintf(stderr, "Erro Aho-Corasick: Autômato não inicializado. Chame ac_initialize_automaton() primeiro.\n");
        return;
    }
    if (ac->num_patterns >= AC_MAX_PATTERNS) {
        fprintf(stderr, "Erro Aho-Corasick: Número máximo de padrões atingido.\n");
        return;
    }
    ac->patterns[ac->num_patterns] = pattern;
    if (aho_trie_add_pattern(ac->trie_automaton_nodes, &ac->current_nodes_count, pattern, ac->num_patterns) == -1) {
        fprintf(stderr, "Erro Aho-Corasick: Número máximo de vértices atingido ao adicionar padrão \"%s\". Aumente AC_MAX_VERTICES.\n", pattern);
    } else {
        ac->num_patterns++;
    }
}

void ac_build_automaton(ac_automaton_t *ac) {
    if (ac->current_nodes_count == 0) return;
    aho_queue_init(&ac->queue);

    for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
        int child_of_root_idx = ac->trie_automaton_nodes[0].next[c];
        if (child_of_root_idx != -1) {
            ac->trie_automaton_nodes[0].go[c] = child_of_root_idx;
            ac->trie_automaton_nodes[child_of_root_idx].link = 0;
            aho_queue_enqueue(&ac->queue, child_of_root_idx);
        } else {
            ac->trie_automaton_nodes[0].go[c] = 0;
        }
    }

    while (!aho_queue_is_empty(&ac->queue)) {
        int u_node_idx = aho_queue_dequeue(&ac->queue);
        for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
            int v_child_idx = ac->trie_automaton_nodes[u_node_idx].next[c];
            if (v_child_idx != -1) {
                ac->trie_automaton_nodes[u_node_idx].go[c] = v_child_idx;
                int u_failure_link_idx = ac->trie_automaton_nodes[u_node_idx].link;
                ac->trie_automaton_nodes[v_child_idx].link = ac->trie_automaton_nodes[u_failure_link_idx].go[c];
                aho_queue_enqueue(&ac->queue, v_child_idx);
            } else {
                int u_failure_link_idx = ac->trie_automaton_nodes[u_node_idx].link;
                ac->trie_automaton_nodes[u_node_idx].go[c] = ac->trie_automaton_nodes[u_failure_link_idx].go[c];
            }
        }
    }
}

void ac_search(ac_automaton_t *ac, const char* text) {
    if (ac->current_nodes_count <= 0 || (ac->trie_automaton_nodes[0].go[0] == -1 && ac->trie_automaton_nodes[0].next[0] == -1 && ac->current_nodes_count == 1)) {
         fprintf(stderr, "Aviso Aho-Corasick: Busca chamada antes de ac_build_automaton() ou nenhum padrão significativo foi adicionado.\n");
    }

    int current_state_idx = 0;

    for (int i = 0; text[i] != '\0'; ++i) {
        char current_char = text[i];
        if (current_char < 'a' || current_char > 'z') continue;
        int char_idx = current_char - 'a';

        if (ac->trie_automaton_nodes[current_state_idx].go[char_idx] == -1 && current_state_idx == 0) {
            ac->trie_automaton_nodes[current_state_idx].go[char_idx] = 0;
        }

        current_state_idx = ac->trie_automaton_nodes[current_state_idx].go[char_idx];

        int temp_state_idx = current_state_idx;
        while (true) {
            if (ac->trie_automaton_nodes[temp_state_idx].output) {
                for (int k = 0; k < ac->trie_automaton_nodes[temp_state_idx].num_patterns; ++k) {
                    int pat_idx = ac->trie_automaton_nodes[temp_state_idx].pattern_indices[k];
                    ac_set_match_callback(ac->patterns[pat_idx], i);
                }
            }
            if (temp_state_idx == 0) break;
            temp_state_idx = ac->trie_automaton_nodes[temp_state_idx].link;
        }
    }
}