#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <malloc.h>
#include "bootselbtn.h"
#include "btnstate.h"
#include "event_queue.h"
#include "stack_queue.h"
#include "app_state.h"
#include "memory_util.h"

QUEUE queue;

void core1_entry()
{
    while (true)
    {
        // If no events, dequeue a stack function
        QUEUE_ITEM func = dequeue(&queue);
        if (func.exists)
        {
            func.work_func();
        }

        sleep_ms(get_tick_sleep_ms());
    }
}

typedef struct
{
    int count;
} ARGS_T;

void sinlge_work_func()
{
    printf("sinlge_work_func, starting work\n");
    sleep_ms(2000);
    printf("sinlge_work_func, work complete\n");
}

void screen_0_load()
{
    if (0 != get_app_screen())
    {
        return;
    }
    enqueue(&queue, sinlge_work_func);
}

void screen_1_load()
{
    if (1 != get_app_screen())
    {
        return;
    }

    enqueue(&queue, sinlge_work_func);
}

void handle_long_button_press()
{
    next_app_screen();

    int app_screen = get_app_screen();

    switch (app_screen)
    {
    case 0:
        screen_0_load();
        break;
    case 1:
        screen_1_load();
        break;
    default:
        printf("handle_long_button_press, invalid screen %d\n", app_screen);
        break;
    }
}

void handle_short_button_press()
{
    printf("handle_short_button_press\n");
}

void main_tick()
{
    EVENT_T event = dequeue_event();

    switch (event)
    {
    case BTN_1_LONG_PRESS:
        handle_long_button_press();
        break;
    case BTN_1_SHORT_PRESS:
        handle_short_button_press();
        break;
    case NONE:
        // Nothing to do, just keep looping
        break;
    default:
        printf("main_tick, invalid event %d\n", event);
        break;
    }
}

int main()
{
    stdio_init_all();

    sleep_ms(5000);

    queue_init(&queue);

    multicore_launch_core1(core1_entry);

    set_awake_tick_sleep_ms();

    screen_0_load();

    while (true)
    {
        BTN_TICK_STATE_T btn_state = btn_state_tick();

        if (btn_state.released)
        {
            if (LONG_PRESS == btn_state.release_type)
            {
                enqueue_event(BTN_1_LONG_PRESS);
            }
            else if (SHORT_PRESS == btn_state.release_type)
            {
                enqueue_event(BTN_1_SHORT_PRESS);
            }
        }

        main_tick();

        print_free_heap();
        sleep_ms(get_tick_sleep_ms());
    }
}
