#include "pico/stdlib.h"

#ifndef _BTN_STATE_H
#define _BTN_STATE_H

#define LONG_HOLD_THRESHOLD_MS 3000
#define LONG_PRESS_THRESHOLD_MS 800

typedef enum
{
  LONG_PRESS,
  SHORT_PRESS,
} BTN_PRESS_TYPE_T;

typedef struct 
{
  uint gpio;
  bool btn_prev_pressed;
  absolute_time_t btn_press_began;
} BTN_T;

typedef struct
{
  bool released;
  BTN_PRESS_TYPE_T release_type;
  bool long_hold;
  bool pressed;
} BTN_TICK_STATE_T;

void init_btn(BTN_T *btn, uint gpio);
BTN_TICK_STATE_T btn_state_tick(BTN_T *btn);

#endif
