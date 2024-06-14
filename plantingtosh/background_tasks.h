#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#ifndef _inc_background_tasks
#define _inc_background_tasks

typedef struct
{
  void (*func)(void);
  int perform_every_ms;
  absolute_time_t last_performed;
  char *name;
} BACKGROUND_TASK_T;

typedef struct
{
    void (*func)(void);
} BACKGROUND_QUEUE_T;

void add_background_task(char *name,  void (*func)(void), int perform_every_ms);

void enqueue_background_tasks(queue_t *queue);

#endif
