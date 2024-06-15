#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "board_id.h"
#include "events.h"
#include "flowering.c"
#include "plant_stats.h"
#include "screen_manager.h"
#include "screen_size.h"
#include "ssd1306.h"
#include "utils.h"

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static const int sleep_screen_id = -1;
static const int boot_screen_id = 0;
static const int plant_stats_screen_id = 1;
static const int info_screen_id = 2;

static const int first_screen_id = plant_stats_screen_id;
static const int last_screen_id = info_screen_id;

static int current_screen = boot_screen_id;

static ssd1306_t disp;
static const uint8_t screen_address = 0x3C;

static int temp_unit = FAHRENHEIT;

void init_screen_manager()
{
  disp.external_vcc = false;
  ssd1306_init(&disp, SCREEN_WIDTH, SCREEN_HEIGHT, screen_address, i2c0);
  ssd1306_clear(&disp);
}

void show_info()
{
  ssd1306_clear(&disp);
  draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[2]);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, FIRST_LINE, 1, "Plant ID");
  ssd1306_draw_string(&disp, X_SPLIT_POINT, SECOND_LINE, 1, get_board_id());
  ssd1306_show(&disp);
}

void info_screen_entry()
{
  show_info();
}

void info_screen_handle_event(EVENT_T event)
{
  if (current_screen != info_screen_id)
  {
    return;
  }

  switch (event)
  {
  default:
    break;
  }
}

void refresh_show_plant_stats()
{
  DEBUG_printf("refresh_show_plant_stats\n");
  PLANT_STATS_T *stats = get_current_stats();

  int temp_tenths = temp_unit == CELCIUS ? stats->temp : (stats->temp * 9) / 5 + 320;

  char *temp_prefix = "Temp  : ";
  // 1 for the decimal point, 1 for space between number and temp_unit, 1 for temp_unit character 1 for null terminator
  char *temp_str = malloc(strlen(temp_prefix) + get_number_length(temp_tenths) + 2 + 1 + 1);
  sprintf(temp_str, "%s%d.%d ", temp_prefix, temp_tenths / 10, temp_tenths % 10);
  if (temp_unit == FAHRENHEIT)
  {
    strcat(temp_str, "F");
  }
  else
  {
    strcat(temp_str, "C");
  }

  char *moisture_prefix = "Moist : ";
  char *moisture_str = malloc(strlen(moisture_prefix) + get_number_length(stats->capacitence) + 1);
  sprintf(moisture_str, "%s%d", moisture_prefix, stats->capacitence);

  ssd1306_clear_square(&disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, FIRST_LINE, 1, temp_str);
  ssd1306_draw_string(&disp, X_SPLIT_POINT, SECOND_LINE, 1, moisture_str);
  ssd1306_show(&disp);

  DEBUG_printf("refresh_show_plant_stats, %s\n", temp_str);
  DEBUG_printf("refresh_show_plant_stats, %s\n", moisture_str);

  free(stats);
  free(temp_str);
  free(moisture_str);

  DEBUG_printf("refresh_plant_status, done\n");
}

absolute_time_t last_jump_time;
bool jumping = false;
int jump_every_ms = 10 * 1000;
int hang_time_ms = 300;

void plant_stats_screen_entry()
{
  jumping = false;
  last_jump_time = get_absolute_time();
  ssd1306_clear(&disp);
  draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
  refresh_show_plant_stats();
}

static void update_jumping_animation()
{
  if (jumping && absolute_time_diff_us(last_jump_time, get_absolute_time()) > hang_time_ms * 1000)
  {
    jumping = false;

    ssd1306_clear_square(&disp, 0, 0, X_SPLIT_POINT, SCREEN_HEIGHT);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
    ssd1306_show(&disp);
  }
  else if (!jumping && absolute_time_diff_us(last_jump_time, get_absolute_time()) > jump_every_ms * 1000)
  {
    jumping = true;
    last_jump_time = get_absolute_time();

    ssd1306_clear_square(&disp, 0, 0, X_SPLIT_POINT, SCREEN_HEIGHT);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[4]);
    ssd1306_show(&disp);
  }
}

void plant_stats_screen_handle_event(EVENT_T event)
{
  if (current_screen != plant_stats_screen_id)
  {
    return;
  }

  switch (event)
  {
  case BTN_1_SHORT_PRESS:
    refresh_show_plant_stats();
    break;
  case BTN_1_LONG_PRESS:
    temp_unit = temp_unit == FAHRENHEIT ? CELCIUS : FAHRENHEIT;
    refresh_show_plant_stats();
  case NONE:
    update_jumping_animation();
    break;
  default:
    break;
  }
}

static void next_screen()
{
  DEBUG_printf("next_screen, current_screen: %d\n", current_screen);

  if (current_screen == last_screen_id)
  {
    current_screen = first_screen_id;
  }
  else
  {
    current_screen++;
  }

  switch (current_screen)
  {
  case plant_stats_screen_id:
    plant_stats_screen_entry();
    break;
  case info_screen_id:
    info_screen_entry();
    break;
  default:
    printf("next_screen, invalid screen %d\n", current_screen);
    break;
  }

  DEBUG_printf("next_screen, new current_screen: %d\n", current_screen);
}

/**
 * Handles deciding if screen is currently asleep or not
 */
static bool handle_sleep_check(EVENT_T event)
{

  if (event == BTN_2_LONG_HOLD)
  {
    if (current_screen == sleep_screen_id)
    {
      current_screen = first_screen_id;
      ssd1306_poweron(&disp);
      plant_stats_screen_entry();
    }
    else
    {
      current_screen = sleep_screen_id;
      ssd1306_poweroff(&disp);
    }
  }

  return current_screen == sleep_screen_id;
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
  default:
    plant_stats_screen_handle_event(event);
    info_screen_handle_event(event);
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
