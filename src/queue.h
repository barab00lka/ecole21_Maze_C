#pragma once

#include "position.h"
#include "utils.h"
#include <stdbool.h>

typedef struct
{
  Position *data;
  int front, rear, size, capacity;
} Queue;

Queue *create_queue (int capacity);
bool is_queue_empty (Queue *queue);
bool is_queue_full (Queue *queue);
void enqueue (Queue *queue, Position item);
Position dequeue (Queue *queue);
void destroy_queue (Queue *queue);