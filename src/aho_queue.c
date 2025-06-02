#include "aho_queue.h"
#include <stdio.h>

void aho_queue_init(aho_queue_t *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

bool aho_queue_enqueue(aho_queue_t *q, int node_idx) {
    if (q->count == AC_MAX_VERTICES) {
        fprintf(stderr, "[ERRO] Fila cheia! Não é possível enfileirar o nó %d.\n", node_idx);
        return false;
    }
    q->queue[q->tail] = node_idx;
    q->tail = (q->tail + 1) % AC_MAX_VERTICES;
    q->count++;
    return true;
}

int aho_queue_dequeue(aho_queue_t *q) {
    if (q->count == 0) {
        fprintf(stderr, "[ERRO] Fila vazia! Não é possível desenfileirar.\n");
        return -1;
    }
    int node_idx = q->queue[q->head];
    q->head = (q->head + 1) % AC_MAX_VERTICES;
    q->count--;
    return node_idx;
}

bool aho_queue_is_empty(const aho_queue_t *q) {
    return q->count == 0;
}