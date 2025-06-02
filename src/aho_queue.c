#include "aho_queue.h"
#include "aho_corasick.h"

static int bfs_internal_queue[AC_MAX_VERTICES];
static int queue_internal_head = 0;
static int queue_internal_tail = 0;

void aho_queue_init(void) {
    queue_internal_head = 0;
    queue_internal_tail = 0;
}

void aho_queue_enqueue(int node_idx) {
    if (queue_internal_tail < AC_MAX_VERTICES) {
        bfs_internal_queue[queue_internal_tail++] = node_idx;
    }
}

int aho_queue_dequeue(void) {
    if (queue_internal_head < queue_internal_tail) {
        return bfs_internal_queue[queue_internal_head++];
    }
    return -1;
}

bool aho_queue_is_empty(void) {
    return queue_internal_head == queue_internal_tail;
}