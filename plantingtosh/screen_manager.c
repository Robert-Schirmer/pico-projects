#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_macros.h"
#include "events.h"
#include "screen_size.h"
#include "ssd1306.h"
#include "flowering.c"
#include "screen_manager.h"
#include "screens/boot_screens.h"
#include "screens/plant_stats_screens.h"
#include "screens/settings_screens.h"
#include "screens/sleep_screens.h"

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static ssd1306_t disp;

static ScreenGroup *current_screen_group = &boot_screen_group;
static Screen *current_screen = &boot_screen;

static const uint8_t screen_address = 0x3C;

void init_screen_manager()
{
  disp.external_vcc = false;
  ssd1306_init(&disp, SCREEN_WIDTH, SCREEN_HEIGHT, screen_address, i2c0);
  ssd1306_clear(&disp);

  settings_screen_group.init(&disp);
  plant_stats_screen_group.init(&disp);
  sleep_screen_group.init(&disp);
}

int is_current_screen(Screen *screen)
{
  return screen->id == current_screen->id;
}

static void next_screen()
{
  DEBUG_printf("next_screen, current_screen: %d\n", current_screen->id);

  if (current_screen->next_screen == NULL)
  {
    DEBUG_printf("next_screen, current_screen does not have a next screen\n");
    return;
  }

  current_screen = current_screen->next_screen;
  current_screen->entry();

  DEBUG_printf("next_screen, new current_screen: %d\n", current_screen->id);
}

void jump_to_screen_group(ScreenGroup *screen_group)
{
  DEBUG_printf("jump_to_screen_group, screen_group: %d\n", screen_group->id);

  current_screen_group = screen_group;
  current_screen = screen_group->first_screen;
  current_screen->entry();

  DEBUG_printf("jump_to_screen_group, new current_screen_group: %d\n", current_screen_group->id);
}

void next_screen_group()
{
  DEBUG_printf("next_screen_group, current_screen_group: %d\n", current_screen_group->id);

  if (current_screen_group->next_screen_group == NULL)
  {
    DEBUG_printf("next_screen_group, current_screen_group does not have a next screen group\n");
    return;
  }

  current_screen_group = current_screen_group->next_screen_group;
  current_screen = current_screen_group->first_screen;
  current_screen->entry();

  DEBUG_printf("next_screen_group, new current_screen_group: %d\n", current_screen_group->id);
}

void handle_event_on_screen(EVENT_T event)
{
  if (event != NONE)
  {
    DEBUG_printf("handle_event_on_screen, event: %d\n", event);
  }

  if (handle_sleep_check(event))
  {
    return;
  }

  switch (event)
  {
  case BTN_2_SHORT_PRESS:
    next_screen();
    break;
  case BTN_2_LONG_PRESS:
    next_screen_group();
    break;
  default:
    if (current_screen->handle_event != NULL)
    {
      current_screen->handle_event(event);
    }
    break;
  }
}

void show_boot_screen_sequence(int sequence_number)
{
  DEBUG_printf("show_boot_screen_sequence, sequence_number: %d\n", sequence_number);
  if (sequence_number == 0)
  {
    ssd1306_clear(&disp);
    ssd1306_draw_string(&disp, X_SPLIT_POINT, SECOND_LINE, 1, "Plantingtosh");
  }
  else
  {
    ssd1306_clear_square(&disp, 0, 0, X_SPLIT_POINT, SCREEN_HEIGHT);
  }
  draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[sequence_number]);
  ssd1306_show(&disp);
}

void show_boot_screen_msg(char *line1, char *line2, char *line3)
{
  DEBUG_printf("show_boot_screen_msg, line1: %s, line2: %s, line3: %s\n", line1, line2, line3);
  ssd1306_clear_square(&disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, FIRST_LINE, 1, line1);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, SECOND_LINE, 1, line2);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, THIRD_LINE, 1, line3);
  ssd1306_show(&disp);
}
