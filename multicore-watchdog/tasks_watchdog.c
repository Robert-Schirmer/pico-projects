#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/watchdog.h"
#include "tasks_watchdog.h"

static bool *tasks_status;
static int task_count;

static void set_tasks_unknown()
{
  for (size_t i = 0; i < task_count; i++)
  {
    tasks_status[i] = false;
  }
}

void init_tasks_watchdog(uint32_t delay_ms, bool pause_on_debug, int _task_count)
{
  tasks_status = malloc(_task_count * sizeof(bool));
  task_count = _task_count;

  set_tasks_unknown();

  watchdog_enable(delay_ms, pause_on_debug);
}

void tasks_watchdog_update()
{
  for (size_t i = 0; i < task_count; i++)
  {
    if (!tasks_status[i])
    {
      printf("Task %d is dead\n", i);
      // Force a watchdog reboot
      watchdog_enable(1, false);
      while (true)
        ;
    }
  }

  set_tasks_unknown();

  watchdog_update();
}

void set_task_alive(int task_num)
{
  tasks_status[task_num] = true;
}
