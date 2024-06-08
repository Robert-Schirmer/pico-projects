#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/mutex.h"
#include "stack_queue.h"

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

void queue_init(QUEUE *queue)
{
    queue->size = QUEUE_SIZE;
    queue->front = -1;
    queue->rear = 0;
    mutex_init(&queue->mutex);
}

int enqueue(QUEUE *queue, void *item)
{
    DEBUG_printf("enqueue\n");
    mutex_enter_blocking(&queue->mutex);
    int new_stack_loop_rear = queue->rear == queue->size - 1 ? 0 : queue->rear + 1;
    if (new_stack_loop_rear == queue->front)
    {
        printf("enqueue QUEUE OVERFLOW, ABORTING\n");
        mutex_exit(&queue->mutex);
        abort();
    }

    queue->rear = new_stack_loop_rear;

    queue->queue[queue->rear].item = item;
    queue->queue[queue->rear].exists = true;
    mutex_exit(&queue->mutex);
    DEBUG_printf("enqueue complete, front %d rear %d\n", queue->front, queue->rear);
    return 1;
}

QUEUE_ITEM empty_item = {NULL, false};

QUEUE_ITEM dequeue(QUEUE *queue)
{
    DEBUG_printf("dequeue\n");
    mutex_enter_blocking(&queue->mutex);
    int new_stack_loop_front = queue->front == queue->size - 1 ? 0 : queue->front + 1;
    if (new_stack_loop_front == queue->rear)
    {
        // Underflow, nothing to dequeue
        mutex_exit(&queue->mutex);
        return empty_item;
    }

    queue->front = new_stack_loop_front;

    // Index into the loop stack one past the front since the front is set to always trail the rear by at least one
    int event_index = queue->front == queue->size - 1 ? 0 : queue->front + 1;
    mutex_exit(&queue->mutex);
    DEBUG_printf("dequeue complete, front %d rear %d\n", queue->front, queue->rear);
    return queue->queue[event_index];
}
