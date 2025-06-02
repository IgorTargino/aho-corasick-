#ifndef AHO_QUEUE_H
#define AHO_QUEUE_H

#include <stdbool.h>
#include "aho_config.h"

/**
 * @file aho_queue.h
 * @brief Fila circular auxiliar para a construção do autômato de Aho-Corasick (BFS).
 */

/**
 * @struct aho_queue_t
 * @brief Estrutura de fila circular para armazenar índices de nós durante a BFS.
 *
 * Utilizada para percorrer o trie/autômato na construção dos links de falha.
 */
typedef struct {
    int queue[AC_MAX_VERTICES]; /**< Vetor circular de índices dos nós. */
    int head;                   /**< Índice do início da fila. */
    int tail;                   /**< Índice do final da fila. */
    int count;                  /**< Número de elementos atualmente na fila. */
} aho_queue_t;

/**
 * @brief Inicializa a fila.
 * @param q Ponteiro para a fila a ser inicializada.
 */
void aho_queue_init(aho_queue_t *q);

/**
 * @brief Enfileira um índice de nó na fila.
 * @param q Ponteiro para a fila.
 * @param node_idx Índice do nó a ser enfileirado.
 * @return true se a operação foi bem-sucedida, false se a fila estiver cheia.
 */
bool aho_queue_enqueue(aho_queue_t *q, int node_idx);

/**
 * @brief Remove e retorna o índice do nó no início da fila.
 * @param q Ponteiro para a fila.
 * @return Índice do nó desenfileirado, ou -1 se a fila estiver vazia.
 */
int aho_queue_dequeue(aho_queue_t *q);

/**
 * @brief Verifica se a fila está vazia.
 * @param q Ponteiro para a fila.
 * @return true se a fila estiver vazia, false caso contrário.
 */
bool aho_queue_is_empty(const aho_queue_t *q);

#endif