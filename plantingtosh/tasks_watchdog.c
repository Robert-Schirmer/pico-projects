#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "tasks_watchdog.h"

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static absolute_time_t *tasks_last_alive;
static int task_count;
static uint32_t watchdog_delay_ms;

void init_tasks_watchdog(uint32_t delay_ms, bool pause_on_debug, int _task_count)
{
  tasks_last_alive = malloc(_task_count * sizeof(absolute_time_t));
  task_count = _task_count;

  for (size_t i = 0; i < task_count; i++)
  {
    set_task_alive(i);
  }

  watchdog_enable(delay_ms, pause_on_debug);
  watchdog_delay_ms = delay_ms;
}

void tasks_watchdog_update()
{
  for (size_t i = 0; i < task_count; i++)
  {
    if (absolute_time_diff_us(tasks_last_alive[i], get_absolute_time()) > watchdog_delay_ms * 1000)
    {
      printf("TASK %d HAS NOT RESPONDED, watchdog inconming\n", i);
      // Force a watchdog reboot
      watchdog_enable(1, false);
      while (true)
        ;
    }
  }

  watchdog_update();
}

void set_task_alive(int task_num)
{
  DEBUG_printf("set_task_alive, task %d is alive\n", task_num);
  tasks_last_alive[task_num] = get_absolute_time();
}
