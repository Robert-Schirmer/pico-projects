#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "tasks_watchdog.h"

int wd_counter __attribute__((section(".unitialized_data")));

void core1_entry()
{
    absolute_time_t death_clock = get_absolute_time();
    int kill_after_ms = 10000;

    while (true)
    {
        set_task_alive(1);
        if (absolute_time_diff_us(death_clock, get_absolute_time()) > kill_after_ms * 1000)
        {
            printf("Core 1 hanging itself\n");
            while (true)
                ;
        }
        sleep_ms(100);
    }
}

int main()
{
    stdio_init_all();

    sleep_ms(5000);

    if (watchdog_caused_reboot())
    {
        printf("Rebooted by Watchdog!\n");
        wd_counter += 1;
    }
    else
    {
        printf("Clean boot\n");
        wd_counter = 0;
    }

    printf("wd_counter: %d\n", wd_counter);

    init_tasks_watchdog(5000, false, 2);

    printf("Starting\n");

    sleep_ms(10);
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    while (true)
    {
        set_task_alive(0);
        tasks_watchdog_update();
    }
}
