#ifndef AHO_TRIE_H
#define AHO_TRIE_H

#include "aho_corasick.h"

/**
 * @file aho_trie.h
 * @brief Funções auxiliares para manipulação do trie do autômato de Aho-Corasick.
 */

/**
 * @brief Inicializa um nó do trie.
 * @param v Ponteiro para o nó (ac_vertex_t) a ser inicializado.
 */
void aho_trie_init(ac_vertex_t* v);

/**
 * @brief Adiciona um padrão ao trie.
 * 
 * Insere o padrão no trie, criando novos nós conforme necessário.
 * 
 * @param trie Vetor de nós do trie.
 * @param nodes_count Ponteiro para o contador de nós já utilizados no trie (será incrementado conforme necessário).
 * @param pattern String do padrão a ser inserido.
 * @param pattern_idx Índice do padrão no vetor de padrões do autômato.
 * @return Índice do nó final do padrão inserido, ou -1 em caso de erro (ex: limite de nós atingido).
 */
int aho_trie_add_pattern(ac_vertex_t* trie, int* nodes_count, const char* pattern, int pattern_idx);

#endif