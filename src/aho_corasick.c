#include "aho_corasick.h"
#include "aho_queue.h"
#include "aho_trie.h"
#include <stdio.h>
#include <string.h>

static ac_vertex_t trie_automaton_nodes[AC_MAX_VERTICES];
static int current_nodes_count = 0;

void ac_initialize_automaton(void) {
    current_nodes_count = 0;
    if (AC_MAX_VERTICES > 0) {
        aho_trie_init(&trie_automaton_nodes[current_nodes_count++]);
    } else {
        fprintf(stderr, "Erro Aho-Corasick: AC_MAX_VERTICES deve ser maior que 0.\n");
        return;
    }
    aho_queue_init();
}

void ac_add_pattern(const char* pattern) {
    if (current_nodes_count == 0) {
        fprintf(stderr, "Erro Aho-Corasick: Autômato não inicializado. Chame ac_initialize_automaton() primeiro.\n");
        return;
    }
    if (aho_trie_add_pattern(trie_automaton_nodes, &current_nodes_count, pattern) == -1) {
        fprintf(stderr, "Erro Aho-Corasick: Número máximo de vértices atingido ao adicionar padrão \"%s\". Aumente AC_MAX_VERTICES.\n", pattern);
    }
}

void ac_build_automaton(void) {
    if (current_nodes_count == 0) return;
    aho_queue_init();

    for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
        int child_of_root_idx = trie_automaton_nodes[0].next[c];
        if (child_of_root_idx != -1) {
            trie_automaton_nodes[0].go[c] = child_of_root_idx;
            trie_automaton_nodes[child_of_root_idx].link = 0;
            aho_queue_enqueue(child_of_root_idx);
        } else {
            trie_automaton_nodes[0].go[c] = 0;
        }
    }

    while (!aho_queue_is_empty()) {
        int u_node_idx = aho_queue_dequeue();
        for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
            int v_child_idx = trie_automaton_nodes[u_node_idx].next[c];
            if (v_child_idx != -1) {
                trie_automaton_nodes[u_node_idx].go[c] = v_child_idx;
                int u_failure_link_idx = trie_automaton_nodes[u_node_idx].link;
                trie_automaton_nodes[v_child_idx].link = trie_automaton_nodes[u_failure_link_idx].go[c];
                aho_queue_enqueue(v_child_idx);
            } else {
                int u_failure_link_idx = trie_automaton_nodes[u_node_idx].link;
                trie_automaton_nodes[u_node_idx].go[c] = trie_automaton_nodes[u_failure_link_idx].go[c];
            }
        }
    }
}

void ac_search(const char* text) {
    if (current_nodes_count <= 0 || (trie_automaton_nodes[0].go[0] == -1 && trie_automaton_nodes[0].next[0] == -1 && current_nodes_count == 1)) {
         fprintf(stderr, "Aviso Aho-Corasick: Busca chamada antes de ac_build_automaton() ou nenhum padrão significativo foi adicionado.\n");
    }

    printf("\nBuscando no texto: \"%s\"\n", text);
    int current_state_idx = 0;

    for (int i = 0; text[i] != '\0'; ++i) {
        char current_char = text[i];
        if (current_char < 'a' || current_char > 'z') continue;
        int char_idx = current_char - 'a';

        if (trie_automaton_nodes[current_state_idx].go[char_idx] == -1 && current_state_idx == 0) {
            trie_automaton_nodes[current_state_idx].go[char_idx] = 0;
        }

        current_state_idx = trie_automaton_nodes[current_state_idx].go[char_idx];

        int temp_state_idx = current_state_idx;
        while (true) {
            if (trie_automaton_nodes[temp_state_idx].output) {
                printf("  -> Padrão encontrado terminando no índice %d do texto (caractere '%c'), estado do autômato: %d.\n",
                       i, text[i], temp_state_idx);
            }
            if (temp_state_idx == 0) break;
            temp_state_idx = trie_automaton_nodes[temp_state_idx].link;
        }
    }
    printf("Busca Aho-Corasick concluída.\n");
}