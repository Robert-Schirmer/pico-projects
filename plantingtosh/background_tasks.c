#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "background_tasks.h"
#include "pico/util/queue.h"

#define MAX_BACKGROUND_TASKS 10

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static BACKGROUND_TASK_T background_tasks[MAX_BACKGROUND_TASKS];
static size_t num_background_tasks = 0;

void add_background_task(char *name, void (*func)(void), int perform_every_ms)
{
  if (num_background_tasks >= MAX_BACKGROUND_TASKS)
  {
    printf("add_background_task, TOO MANY BACKGROUND TASKS, ABORTING\n");
    abort();
  }
  background_tasks[num_background_tasks].func = func;
  background_tasks[num_background_tasks].perform_every_ms = perform_every_ms;
  background_tasks[num_background_tasks].last_performed = get_absolute_time();
  background_tasks[num_background_tasks].name = name;

  DEBUG_printf("add_background_task, added '%s'\n", name);

  num_background_tasks++;
}

void enqueue_background_tasks(queue_t *queue)
{
  for (size_t i = 0; i < num_background_tasks; i++)
  {
    if (absolute_time_diff_us(background_tasks[i].last_performed, get_absolute_time()) < background_tasks[i].perform_every_ms * 1000)
    {
      continue;
    }
    background_tasks[i].last_performed = get_absolute_time();
    DEBUG_printf("enqueue_background_tasks, '%s'\n", background_tasks[i].name);
    BACKGROUND_QUEUE_T entry = {background_tasks[i].func};
    queue_try_add(queue, &entry);
  }
}
