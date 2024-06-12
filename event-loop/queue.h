#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/mutex.h"

#ifndef _CIRCLE_BUFFER_QUEUE_H
#define _CIRCLE_BUFFER_QUEUE_H

typedef struct
{
  void *item;
  bool exists;
} QUEUE_ITEM;

typedef struct
{
  QUEUE_ITEM *queue;
  int front;
  int rear;
  int size;
  mutex_t mutex;
} QUEUE;

void queue_init(QUEUE *queue, int size);

int enqueue(QUEUE *queue, void *item);
QUEUE_ITEM dequeue(QUEUE *queue);

#endif
