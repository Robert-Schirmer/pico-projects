#include <stdio.h>
#include "pico/stdlib.h"
#include "btnstate.h"
#include "hardware/gpio.h"

void init_btn(BTN_T *btn, uint gpio)
{
  btn->gpio = gpio;
  btn->btn_prev_pressed = false;

  gpio_init(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  gpio_pull_up(gpio);
}

BTN_TICK_STATE_T btn_state_tick(BTN_T *btn)
{
  BTN_TICK_STATE_T btn_state = {false, 0, false, false};
  btn_state.pressed = !gpio_get(btn->gpio);

  int elapsed_since_press_us = absolute_time_diff_us(btn->btn_press_began, get_absolute_time());

  if (btn->btn_prev_pressed && !btn_state.pressed)
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
  else if (!btn->btn_prev_pressed && btn_state.pressed)
  {
    // Just started pressing
    btn->btn_press_began = get_absolute_time();
  }
  else if (btn_state.pressed && elapsed_since_press_us > LONG_HOLD_THRESHOLD_MS * 1000)
  {
    btn_state.long_hold = true;
  }

  btn->btn_prev_pressed = btn_state.pressed;

  return btn_state;
}
