#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/mutex.h"

#ifndef _CIRCLE_BUFFER_QUEUE_H
#define _CIRCLE_BUFFER_QUEUE_H

#define QUEUE_SIZE 5

typedef struct
{
  void (*work_func)();
  bool exists;
} QUEUE_ITEM;

typedef struct
{
  QUEUE_ITEM queue[QUEUE_SIZE];
  int front;
  int rear;
  int size;
  mutex_t mutex;
} QUEUE;

void queue_init(QUEUE *queue);

int enqueue(QUEUE *queue, void (*work_func)(void));
QUEUE_ITEM dequeue(QUEUE *queue);

#endif
