#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../screen_manager.h"
#include "../ssd1306.h"
#include "plant_stats_screens.h"
#include "screen_ids.h"

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

Screen sleep_screen;
void sleep_screen_entry();

void sleep_screens_init(ssd1306_t *display);

bool handle_sleep_check(EVENT_T event);

ScreenGroup sleep_screen_group = {
    .id = SLEEP_SCREEN_GROUP_ID,
    .first_screen = &sleep_screen,
    .next_screen_group = &plant_stats_screen_group,
    .init = sleep_screens_init,
};

Screen sleep_screen = {
    .id = SLEEP_SCREEN_ID,
    .entry = sleep_screen_entry,
    .handle_event = NULL,
    .next_screen = NULL,
};

static ssd1306_t *disp;

void sleep_screens_init(ssd1306_t *display)
{
  DEBUG_printf("sleep_screens_init\n");
  disp = display;
}

void sleep_screen_entry()
{
  DEBUG_printf("sleep_screen_entry\n");
  ssd1306_poweroff(disp);
}

/**
 * Handles deciding if screen is currently asleep or not
 * @returns true if screen is asleep, false otherwise
 */
bool handle_sleep_check(EVENT_T event)
{
  if (event == BTN_2_LONG_HOLD)
  {
    if (is_current_screen(&sleep_screen))
    {
      ssd1306_poweron(disp);
      next_screen_group();
    }
    else
    {
      jump_to_screen_group(&sleep_screen_group);
    }
  }

  return is_current_screen(&sleep_screen);
}
