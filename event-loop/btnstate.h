#ifndef _BTN_STATE_H
#define _BTN_STATE_H

typedef enum
{
  LONG_PRESS,
  SHORT_PRESS,
} BTN_PRESS_TYPE_T;

typedef struct
{
  bool released;
  BTN_PRESS_TYPE_T release_type;
  bool pressed;
} BTN_TICK_STATE_T;

BTN_TICK_STATE_T btn_state_tick();

#endif
