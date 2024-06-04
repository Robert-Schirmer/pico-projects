#include <stdio.h>
#include "pico/stdlib.h"
#include "btnstate.h"
#include "bootselbtn.h"

bool btn_prev_pressed = false;
absolute_time_t btn_press_began;

BTN_TICK_STATE_T btn_state_tick(void)
{
  BTN_TICK_STATE_T btn_state = {false, 0, false};

  btn_state.pressed = get_bootsel_button();

  if (btn_prev_pressed && !btn_state.pressed)
  {
    if (absolute_time_diff_us(btn_press_began, get_absolute_time()) < 300000)
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
    btn_press_began = get_absolute_time();
  }

  btn_prev_pressed = btn_state.pressed;

  return btn_state;
}
