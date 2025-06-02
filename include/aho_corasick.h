#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stdbool.h>
#include "aho_config.h"
#include "aho_queue.h"

/**
 * @file aho_corasick.h
 * @brief Interface do autômato de Aho-Corasick para busca eficiente de múltiplos padrões.
 */

/**
 * @struct ac_vertex_t
 * @brief Representa um nó do trie/autômato de Aho-Corasick.
 *
 * Cada nó armazena transições, links de falha, informações de saída e índices dos padrões que terminam neste nó.
 */
typedef struct {
    int next[AC_K_ALPHABET_SIZE];    
    int go[AC_K_ALPHABET_SIZE];
    int link;
    bool output;
    int pattern_indices[AC_MAX_PATTERNS_PER_NODE];
    int num_patterns;
} ac_vertex_t;

/**
 * @struct ac_automaton_t
 * @brief Estrutura principal do autômato de Aho-Corasick.
 *
 * Armazena todos os nós do trie/automato, padrões, contadores e a fila auxiliar usada na construção.
 */
typedef struct {
    ac_vertex_t trie_automaton_nodes[AC_MAX_VERTICES];
    int current_nodes_count;
    const char* patterns[AC_MAX_PATTERNS];
    int num_patterns;
    aho_queue_t queue; 
} ac_automaton_t;

/**
 * @brief Função de callback chamada quando um padrão é encontrado durante a busca.
 * @param pattern Ponteiro para o padrão encontrado.
 * @param pos Posição no texto onde o padrão termina.
 * @param pattern_index Índice do padrão no vetor de padrões.
 */
extern void ac_set_match_callback(const char* pattern, int pos);

/**
 * @brief Inicializa a estrutura do autômato de Aho-Corasick.
 * @param ac Ponteiro para a estrutura do autômato a ser inicializada.
 */
void ac_initialize_automaton(ac_automaton_t *ac);

/**
 * @brief Adiciona um padrão ao autômato.
 * @param ac Ponteiro para o autômato.
 * @param pattern Padrão a ser adicionado (string terminada em '\0').
 */
void ac_add_pattern(ac_automaton_t *ac, const char* pattern);

/**
 * @brief Constrói o autômato de Aho-Corasick após todos os padrões terem sido adicionados.
 * @param ac Ponteiro para o autômato.
 */
void ac_build_automaton(ac_automaton_t *ac);

/**
 * @brief Realiza a busca dos padrões no texto informado.
 * @param ac Ponteiro para o autômato.
 * @param text Texto onde os padrões serão buscados.
 */
void ac_search(ac_automaton_t *ac, const char* text);

#endif