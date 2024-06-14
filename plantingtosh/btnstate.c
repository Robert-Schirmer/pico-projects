#include <stdio.h>
#include "pico/stdlib.h"
#include "btnstate.h"
#include "hardware/gpio.h"

static bool btn_prev_pressed = false;
static absolute_time_t btn_press_began;

BTN_TICK_STATE_T btn_state_tick(uint gpio)
{
  BTN_TICK_STATE_T btn_state = {false, 0, false, false};
  btn_state.pressed = !gpio_get(gpio);

  int elapsed_since_press_us = absolute_time_diff_us(btn_press_began, get_absolute_time());

  if (btn_prev_pressed && !btn_state.pressed)
  {
    // Released
    if (elapsed_since_press_us < LONG_PRESS_THRESHOLD_MS * 1000)
    {
      btn_state.release_type = SHORT_PRESS;
    }
    else
    {
      btn_state.release_type = LONG_PRESS;
    }

    btn_state.released = true;
  }
  else if (!btn_prev_pressed && btn_state.pressed)
  {
    // Just started pressing
    btn_press_began = get_absolute_time();
  }
  else if (btn_state.pressed && elapsed_since_press_us > LONG_HOLD_THRESHOLD_MS * 1000)
  {
    btn_state.long_hold = true;
  }

  btn_prev_pressed = btn_state.pressed;

  return btn_state;
}
