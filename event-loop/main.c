#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <malloc.h>
#include "bootselbtn.h"
#include "btnstate.h"
#include "queue.h"
#include "app_state.h"
#include "memory_util.h"

QUEUE stack_queue;
QUEUE event_queue;

typedef enum
{
    /**
     * Signals that there are no events in the queue
     */
    NONE,
    BTN_1_SHORT_PRESS,
    BTN_1_LONG_PRESS,
} EVENT_T;

void sinlge_work_func0(void)
{
    printf("sinlge_work_func0, starting work\n");
    sleep_ms(2000);
    printf("sinlge_work_func0, work complete\n");
}

void sinlge_work_func1(void)
{
    printf("sinlge_work_func1, starting work\n");
    sleep_ms(2000);
    printf("sinlge_work_func1, work complete\n");
}

void core1_entry()
{
    while (true)
    {
        // If no events, dequeue a stack function
        QUEUE_ITEM func = dequeue(&stack_queue);
        if (func.exists)
        {
            printf("core1_entry, %p\n", func.item);
            ((void (*)(void))func.item)();
        }

        sleep_ms(get_tick_sleep_ms());
    }
}

typedef struct
{
    int count;
} ARGS_T;

void screen_0_load()
{
    if (0 != get_app_screen())
    {
        return;
    }

    enqueue(&stack_queue, sinlge_work_func0);
}

void screen_1_load()
{
    if (1 != get_app_screen())
    {
        return;
    }

    enqueue(&stack_queue, sinlge_work_func1);
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
    EVENT_T event = NONE;

    QUEUE_ITEM item = dequeue(&event_queue);
    if (item.exists)
    {
        event = *(EVENT_T *)item.item;
    }

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

    // Program won't work with a sleep any lower than this
    sleep_ms(5000);
    printf("Starting\n");

    queue_init(&stack_queue, 10);
    queue_init(&event_queue, 10);

    sleep_ms(10);
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    set_awake_tick_sleep_ms();

    screen_0_load();

    int btn_1_long_press = BTN_1_LONG_PRESS;
    int btn_1_short_press = BTN_1_SHORT_PRESS;

    while (true)
    {
        BTN_TICK_STATE_T btn_state = btn_state_tick();

        if (btn_state.released)
        {
            if (LONG_PRESS == btn_state.release_type)
            {
                enqueue(&event_queue, &btn_1_long_press);
            }
            else if (SHORT_PRESS == btn_state.release_type)
            {
                enqueue(&event_queue, &btn_1_short_press);
            }
        }

        main_tick();

        print_free_heap();
        sleep_ms(get_tick_sleep_ms());
    }
}
