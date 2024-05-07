#ifndef DEQUE_H
#define DEQUE_H

#include "top_level.h"
#include "memory_layer_structs.h"

// DEFINING INTERFACE FUNCTIONS FOR BUILD PROCESS

// Functions to export:

Deque * init_deque();
void destroy_deque(Deque * deque);
int enqueue(Deque * deque, uint64_t item);
int enqueue_front(Deque * deque, uint64_t item);
int dequeue(Deque * deque, uint64_t * ret_item);
int dequeue_rear(Deque * deque, uint64_t * ret_item);
bool is_deque_empty(Deque * deque);

#endif