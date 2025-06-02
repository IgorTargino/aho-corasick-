#ifndef AHO_QUEUE_H
#define AHO_QUEUE_H

#include <stdbool.h>

void aho_queue_init(void);
void aho_queue_enqueue(int node_idx);
int aho_queue_dequeue(void);
bool aho_queue_is_empty(void);

#endif 