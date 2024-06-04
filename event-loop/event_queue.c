#include <stdio.h>
#include <stdint.h>
#include "event_queue.h"
#include <stdlib.h>

#define MAX_EVENT_STACK_SIZE 20

// Front should always trail the rear by at least one
// To check that the front is never passing the rear
int event_stack_rear = 0;
int event_stack_front = -1;

EVENT_T event_loop_stack[MAX_EVENT_STACK_SIZE + 3];

int enqueue_event(EVENT_T event)
{
    int new_event_stack_rear = event_stack_rear == MAX_EVENT_STACK_SIZE - 1 ? 0 : event_stack_rear + 1;
    if (new_event_stack_rear == event_stack_front)
    {
        printf("enqueue_event_loop queue overflow\n");
        abort();
    }

    event_stack_rear = new_event_stack_rear;

    event_loop_stack[event_stack_rear] = event;
    return 1;
}

EVENT_T dequeue_event()
{
    int new_event_stack_front = event_stack_front == MAX_EVENT_STACK_SIZE - 1 ? 0 : event_stack_front + 1;
    if (new_event_stack_front == event_stack_rear)
    {
        // Underflow, nothing to dequeue
        return NONE;
    }

    event_stack_front = new_event_stack_front;

    // Index into the loop stack one past the front since the front is set to always trail the rear by at least one
    int event_index = event_stack_front == MAX_EVENT_STACK_SIZE - 1 ? 0 : event_stack_front + 1;
    return event_loop_stack[event_index];
}
