#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plant_stats_screens.h"
#include "settings_screens.h"
#include "screen_ids.h"
#include "../plant_stats.h"
#include "../flowering.c"
#include "../screen_size.h"
#include "../ssd1306.h"
#include "../utils.h"

void plant_temp_screen_entry();
void plant_temp_screen_handle_event(EVENT_T event);

void plant_moisture_screen_entry();
void plant_moisture_screen_handle_event(EVENT_T event);

Screen plant_temp_screen;
Screen plant_moisture_screen;

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static ssd1306_t *disp;

void plant_stats_screens_init(ssd1306_t *display)
{
  DEBUG_printf("plant_stats_screens_init\n");
  disp = display;
}

ScreenGroup plant_stats_screen_group = {
    .id = PLANT_STATS_SCREEN_GROUP_ID,
    .first_screen = &plant_temp_screen,
    .next_screen_group = &settings_screen_group,
    .init = plant_stats_screens_init,
};

Screen plant_temp_screen = {
    .id = PLANT_TEMP_SCREEN_ID,
    .entry = plant_temp_screen_entry,
    .handle_event = plant_temp_screen_handle_event,
    .next_screen = &plant_moisture_screen,
};

Screen plant_moisture_screen = {
    .id = PLANT_MOISTURE_SCREEN_ID,
    .entry = plant_moisture_screen_entry,
    .handle_event = plant_moisture_screen_handle_event,
    .next_screen = &plant_temp_screen,
};

absolute_time_t last_jump_time;
bool jumping = false;
int jump_every_ms = 10 * 1000;
int hang_time_ms = 300;

static int temp_unit = FAHRENHEIT;

static void update_jumping_animation()
{
  if (jumping && absolute_time_diff_us(last_jump_time, get_absolute_time()) > hang_time_ms * 1000)
  {
    jumping = false;

    ssd1306_clear_square(disp, 0, 0, X_SPLIT_POINT, SCREEN_HEIGHT);
    draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
    ssd1306_show(disp);
  }
  else if (!jumping && absolute_time_diff_us(last_jump_time, get_absolute_time()) > jump_every_ms * 1000)
  {
    jumping = true;
    last_jump_time = get_absolute_time();

    ssd1306_clear_square(disp, 0, 0, X_SPLIT_POINT, SCREEN_HEIGHT);
    draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[4]);
    ssd1306_show(disp);
  }
}

void refresh_show_temp()
{
  DEBUG_printf("refresh_show_temp\n");
  PLANT_STATS_T *stats = get_current_stats();

  int temp_tenths = temp_unit == CELCIUS ? stats->temp : (stats->temp * 9) / 5 + 320;

  // 1 for the decimal point, 1 for space between number and temp_unit, 1 for temp_unit character 1 for null terminator
  char *temp_str = malloc(get_number_length(temp_tenths) + 2 + 1 + 1);
  sprintf(temp_str, "%d.%d ", temp_tenths / 10, temp_tenths % 10);
  if (temp_unit == FAHRENHEIT)
  {
    strcat(temp_str, "F");
  }
  else
  {
    strcat(temp_str, "C");
  }

  ssd1306_clear_square(disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
  ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Temp");
  ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 2, temp_str);
  ssd1306_show(disp);

  DEBUG_printf("refresh_show_temp, %s\n", temp_str);

  free(stats);
  free(temp_str);

  DEBUG_printf("refresh_show_temp, done\n");
}

void refresh_show_moisture()
{
  DEBUG_printf("refresh_show_moisture\n");
  PLANT_STATS_T *stats = get_current_stats();

  char *moisture_str = malloc(get_number_length(stats->capacitence) + 1);
  sprintf(moisture_str, "%d", stats->capacitence);

  ssd1306_clear_square(disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
  ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Moisture");
  ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 2, moisture_str);
  ssd1306_show(disp);

  DEBUG_printf("refresh_show_moisture, %s\n", moisture_str);

  free(stats);
  free(moisture_str);

  DEBUG_printf("refresh_show_moisture, done\n");
}

void plant_temp_screen_entry()
{
  jumping = false;
  last_jump_time = get_absolute_time();
  ssd1306_clear(disp);
  draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
  refresh_show_temp();
}

void plant_moisture_screen_entry()
{
  jumping = false;
  last_jump_time = get_absolute_time();
  ssd1306_clear(disp);
  draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
  refresh_show_moisture();
}

void plant_temp_screen_handle_event(EVENT_T event)
{
  switch (event)
  {
  case BTN_1_SHORT_PRESS:
    refresh_show_temp();
    break;
  case BTN_1_LONG_PRESS:
    temp_unit = temp_unit == FAHRENHEIT ? CELCIUS : FAHRENHEIT;
    refresh_show_temp();
  case NONE:
    update_jumping_animation();
    break;
  default:
    break;
  }
}

void plant_moisture_screen_handle_event(EVENT_T event)
{
  switch (event)
  {
  case BTN_1_SHORT_PRESS:
    refresh_show_moisture();
    break;
  case NONE:
    update_jumping_animation();
    break;
  default:
    break;
  }
}
