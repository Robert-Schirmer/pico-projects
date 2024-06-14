#include <stdio.h>
#include <stdint.h>

#ifndef _TASK_WATCHDOG_H
#define _TASK_WATCHDOG_H

void init_tasks_watchdog(uint32_t delay_ms, bool pause_on_debug, int _task_count);
void tasks_watchdog_update(void);
void set_task_alive(int task_num);

#endif
