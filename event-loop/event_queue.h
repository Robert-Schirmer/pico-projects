#ifndef _EVENT_QUEUE_H
#define _EVENT_QUEUE_H

typedef enum
{
  /**
   * Signals that there are no events in the queue
   */
  NONE,
  BTN_1_SHORT_PRESS,
  BTN_1_LONG_PRESS,
} EVENT_T;

int enqueue_event(EVENT_T event);
EVENT_T dequeue_event(void);

#endif
