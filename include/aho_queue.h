#ifndef AHO_QUEUE_H
#define AHO_QUEUE_H

#include <stdbool.h>
#include "aho_config.h"

typedef struct {
    int queue[AC_MAX_VERTICES];
    int head;
    int tail;
    int count;
} aho_queue_t;

void aho_queue_init(aho_queue_t *q);
bool aho_queue_enqueue(aho_queue_t *q, int node_idx);
int aho_queue_dequeue(aho_queue_t *q);
bool aho_queue_is_empty(const aho_queue_t *q);

#endif