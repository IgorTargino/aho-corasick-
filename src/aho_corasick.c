#include "aho_corasick.h"
#include <stdio.h>   // Para printf, fprintf, stderr
#include <string.h>  // Para strlen (usado em ac_add_pattern para iteração, embora não estritamente)
                     // e para outras funções de string se necessário.

// --- Variáveis Estáticas Internas ao Módulo (Estado do Autômato) ---

// Array estático para os nós do autômato.
static ac_vertex_t trie_automaton_nodes[AC_MAX_VERTICES];
static int current_nodes_count = 0; // Contador de nós utilizados

// Fila para BFS (usada na construção dos links de falha)
// Tamanho máximo para a fila BFS, não pode ser maior que o total de nós.
#define AC_MAX_QUEUE_SIZE AC_MAX_VERTICES
static int bfs_internal_queue[AC_MAX_QUEUE_SIZE];
static int queue_internal_head = 0;
static int queue_internal_tail = 0;

// --- Funções Auxiliares Internas ---

// Inicializa um novo vértice/nó
static void init_ac_vertex(ac_vertex_t* v) {
    if (v == NULL) return;
    v->link = 0; // Link de falha padrão (para a raiz), será recalculado
    v->output = false;
    for (int i = 0; i < AC_K_ALPHABET_SIZE; ++i) {
        v->next[i] = -1; // -1 indica ausência de transição
        v->go[i] = -1;   // -1 indica que a transição 'go' não foi calculada/definida ainda
    }
}

// Funções da Fila BFS (Estáticas, internas a este arquivo)
static void internal_enqueue(int node_idx) {
    if (queue_internal_tail < AC_MAX_QUEUE_SIZE) {
        bfs_internal_queue[queue_internal_tail++] = node_idx;
    } else {
        fprintf(stderr, "Erro interno Aho-Corasick: Fila BFS cheia!\n");
    }
}

static int internal_dequeue(void) {
    if (queue_internal_head < queue_internal_tail) {
        return bfs_internal_queue[queue_internal_head++];
    }
    return -1; // Fila vazia
}

static bool internal_is_queue_empty(void) {
    return queue_internal_head == queue_internal_tail;
}

// --- Implementação das Funções Públicas ---

void ac_initialize_automaton(void) {
    current_nodes_count = 0;
    // "Cria" e inicializa o nó raiz (estado 0)
    if (AC_MAX_VERTICES > 0) { // Proteção mínima
        init_ac_vertex(&trie_automaton_nodes[current_nodes_count++]);
    } else {
        fprintf(stderr, "Erro Aho-Corasick: AC_MAX_VERTICES deve ser maior que 0.\n");
        return;
    }
    
    // Reseta a fila BFS
    queue_internal_head = 0;
    queue_internal_tail = 0;
}

void ac_add_pattern(const char* pattern) {
    if (current_nodes_count == 0) {
        fprintf(stderr, "Erro Aho-Corasick: Autômato não inicializado. Chame ac_initialize_automaton() primeiro.\n");
        return;
    }
    int current_node_idx = 0; // Começa na raiz (estado 0)
    for (int i = 0; pattern[i] != '\0'; ++i) {
        char ch = pattern[i];
        if (ch < 'a' || ch > 'z') continue; // Processa apenas 'a'-'z'
        int char_idx = ch - 'a';

        if (trie_automaton_nodes[current_node_idx].next[char_idx] == -1) {
            if (current_nodes_count >= AC_MAX_VERTICES) {
                fprintf(stderr, "Erro Aho-Corasick: Número máximo de vértices (%d) atingido ao adicionar padrão \"%s\". Aumente AC_MAX_VERTICES.\n",
                          AC_MAX_VERTICES, pattern);
                return;
            }
            trie_automaton_nodes[current_node_idx].next[char_idx] = current_nodes_count;
            init_ac_vertex(&trie_automaton_nodes[current_nodes_count]);
            current_nodes_count++;
        }
        current_node_idx = trie_automaton_nodes[current_node_idx].next[char_idx];
    }
    trie_automaton_nodes[current_node_idx].output = true;
}

void ac_build_automaton(void) {
    if (current_nodes_count == 0) {
        // Não há nada para construir se não foi inicializado ou nenhum padrão adicionado
        return;
    }
    // Reseta a fila para garantir
    queue_internal_head = 0; 
    queue_internal_tail = 0;

    // Nó raiz (estado 0): link para si mesmo (já é 0 por init_ac_vertex).
    // Calcula transições 'go' da raiz e enfileira filhos diretos.
    for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
        int child_of_root_idx = trie_automaton_nodes[0].next[c];
        if (child_of_root_idx != -1) {
            trie_automaton_nodes[0].go[c] = child_of_root_idx;
            trie_automaton_nodes[child_of_root_idx].link = 0; // Filhos da raiz têm link para a raiz
            internal_enqueue(child_of_root_idx);
        } else {
            trie_automaton_nodes[0].go[c] = 0; // Se não há transição, permanece na raiz
        }
    }

    // Loop BFS para processar os nós
    while (!internal_is_queue_empty()) {
        int u_node_idx = internal_dequeue();

        for (int c = 0; c < AC_K_ALPHABET_SIZE; ++c) {
            int v_child_idx = trie_automaton_nodes[u_node_idx].next[c];

            if (v_child_idx != -1) { // Se u tem um filho direto 'v_child_idx'
                trie_automaton_nodes[u_node_idx].go[c] = v_child_idx;
                int u_failure_link_idx = trie_automaton_nodes[u_node_idx].link;
                trie_automaton_nodes[v_child_idx].link = trie_automaton_nodes[u_failure_link_idx].go[c];
                internal_enqueue(v_child_idx);
            } else { // Se não há filho direto
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
    int current_state_idx = 0; // Começa no estado raiz (0)

    for (int i = 0; text[i] != '\0'; ++i) {
        char current_char = text[i];
        if (current_char < 'a' || current_char > 'z') {
            // Opção: resetar para a raiz se um caractere "inválido" for encontrado
            // current_state_idx = 0; 
            continue; // Ignora caracteres não alfabéticos (minúsculos)
        }
        int char_idx = current_char - 'a';

        // Transita para o próximo estado usando 'go'
        // É importante que 'go' tenha sido preenchido por ac_build_automaton
        if (trie_automaton_nodes[current_state_idx].go[char_idx] == -1 && current_state_idx == 0) {
             // Caso especial: se a raiz não tem transição go para char_idx (deveria ser 0)
             // Isso pode indicar que build_automaton não foi chamado ou falhou
             trie_automaton_nodes[current_state_idx].go[char_idx] = 0; // Failsafe
        } else if (trie_automaton_nodes[current_state_idx].go[char_idx] == -1) {
            // Se um nó não raiz tem go[-1], algo está errado com build_automaton
            // Isso não deveria acontecer se build_automaton foi chamado corretamente.
            // Por segurança, podemos tentar recalcular ou ir para a raiz.
            // Para simplificar, assumimos que build_automaton preencheu 'go' corretamente.
            // Se não, o comportamento é indefinido, mas deveria ser `go[link[estado]][char]`
        }


        current_state_idx = trie_automaton_nodes[current_state_idx].go[char_idx];

        // Verifica correspondências no estado atual e via links de falha
        int temp_state_idx = current_state_idx;
        while (true) { 
            if (trie_automaton_nodes[temp_state_idx].output) {
                printf("  -> Padrão encontrado terminando no índice %d do texto (caractere '%c'), estado do autômato: %d.\n",
                       i, text[i], temp_state_idx);
            }
            if (temp_state_idx == 0) break; // Chegou na raiz (e processou)
            temp_state_idx = trie_automaton_nodes[temp_state_idx].link;
        }
    }
    printf("Busca Aho-Corasick concluída.\n");
}