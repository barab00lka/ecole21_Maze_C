#include "queue.h"

Queue *
create_queue (int capacity)
{
  Queue *queue = (Queue *)safe_malloc (sizeof (Queue));
  queue->capacity = capacity;
  queue->front = queue->size = 0;
  queue->rear = capacity - 1;
  queue->data = (Position *)safe_malloc (capacity * sizeof (Position));
  return queue;
}

bool
is_queue_empty (Queue *queue)
{
  return (queue->size == 0);
}

bool
is_queue_full (Queue *queue)
{
  return (queue->size == queue->capacity);
}

void
enqueue (Queue *queue, Position item)
{
  if (is_queue_full (queue))
    return;
  queue->rear = (queue->rear + 1) % queue->capacity;
  queue->data[queue->rear] = item;
  queue->size++;
}

Position
dequeue (Queue *queue)
{
  if (is_queue_empty (queue))
    {
      Position invalid = { -1, -1 };
      return invalid;
    }

  Position item = queue->data[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size--;
  return item;
}

void
destroy_queue (Queue *queue)
{
  if (queue)
    {
      free (queue->data);
      free (queue);
    }
}
