#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <malloc.h>
#include "bootselbtn.h"
#include "btnstate.h"
#include "btnstate.h"
#include "stack_queue.h"
#include "memory_util.h"

QUEUE stacks_queue;
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

void core1_entry()
{
    while (true)
    {
        // If no events, dequeue a stack function
        QUEUE_ITEM stack = dequeue(&stacks_queue);
        if (stack.exists)
        {
            void *execute_func = stack.item;
            ((void (*)(void))execute_func)();
        }

       sleep_ms(1000);
    }
}

void test_func()
{
    printf("test_func\n");
}

void handle_long_button_press()
{
    printf("handle_long_button_press\n");
}

void handle_short_button_press()
{
    printf("handle_short_button_press\n");
    enqueue(&stacks_queue, &test_func);
}

void main_tick()
{
    QUEUE_ITEM event = dequeue(&event_queue);
    int e = event.exists ? *((int *)event.item) : NONE;

    switch (e)
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

    queue_init(&stacks_queue, 10);
    queue_init(&event_queue, 5);

    printf("Starting core\n");
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(10);
    multicore_launch_core1(core1_entry);
    // Wait for core to start
    sleep_ms(5000);

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
        sleep_ms(100);
    }
}
