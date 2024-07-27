#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings_screens.h"
#include "plant_stats_screens.h"
#include "screen_ids.h"
#include "../flowering.c"
#include "../screen_size.h"
#include "../tcp_client.h"
#include "../app_macros.h"
#include "../app_queues.h"
#include "../background_tasks.h"
#include "../screen_manager.h"
#include "../board_id.h"

void info_screen_entry();
void info_screen_handle_event(EVENT_T event);

void ping_screen_entry();
void ping_screen_handle_event(EVENT_T event);

Screen info_screen;
Screen ping_screen;

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static ssd1306_t *disp;

void settings_screens_init(ssd1306_t *display)
{
  DEBUG_printf("settings_screens_init\n");
  disp = display;
}

ScreenGroup settings_screen_group = {
    .id = SETTINGS_SCREEN_GROUP_ID,
    .first_screen = &info_screen,
    .next_screen_group = &plant_stats_screen_group,
    .init = settings_screens_init,
};

Screen info_screen = {
    .id = INFO_SCREEN_ID,
    .entry = info_screen_entry,
    .handle_event = info_screen_handle_event,
    .next_screen = &ping_screen,
};

Screen ping_screen = {
    .id = PING_SCREEN_ID,
    .entry = ping_screen_entry,
    .handle_event = ping_screen_handle_event,
    .next_screen = &info_screen,
};

void show_info()
{
  ssd1306_clear(disp);
  draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[5]);
  ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Plant ID");
  ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, get_board_id());
  ssd1306_show(disp);
}

void info_screen_entry()
{
  show_info();
}

void info_screen_handle_event(EVENT_T event)
{
  switch (event)
  {
  default:
    break;
  }
}

void ping_server_callback(TCP_SERVER_RESPONSE_T *res)
{
  printf("ping_server_callback, request complete, success: %d\n", res->success);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

  if (is_current_screen(&ping_screen))
  {
    ssd1306_clear_square(disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
    ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Connection");

    if (res->success)
    {
      char *msg = "- success";
      ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, msg);
      ssd1306_draw_string(disp, X_SPLIT_POINT, THIRD_LINE, 1, res->data);
    }
    else
    {
      char *msg = "- failed";
      ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, msg);
      char error_code[2];
      sprintf(error_code, "%d", res->status);
      ssd1306_draw_string(disp, X_SPLIT_POINT, THIRD_LINE, 1, error_code);
    }

    ssd1306_show(disp);
  }

  free_response(res);
}

void ping_server()
{
  char *msg = "PING\n";
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  send_to_server(msg, ping_server_callback);
}

void test_server_connection()
{
  ssd1306_clear_square(disp, X_SPLIT_POINT, 0, SCREEN_WIDTH - X_SPLIT_POINT, SCREEN_HEIGHT);
  ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Connection");
  ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, "- pinging...");
  ssd1306_show(disp);

  BACKGROUND_QUEUE_T entry = {ping_server};
  queue_try_add(&background_queue, &entry);
}

void ping_screen_entry()
{
  ssd1306_clear(disp);
  draw_frame(disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[5]);
  ssd1306_draw_string(disp, X_SPLIT_POINT, FIRST_LINE, 1, "Connection");
  if (WIFI_ENABLED)
  {
    ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, "Press btn to");
    ssd1306_draw_string(disp, X_SPLIT_POINT, THIRD_LINE, 1, "ping server");
  }
  else
  {
    ssd1306_draw_string(disp, X_SPLIT_POINT, SECOND_LINE, 1, "Wifi disabled");
  }
  ssd1306_show(disp);
}

void ping_screen_handle_event(EVENT_T event)
{
  if (!WIFI_ENABLED)
  {
    return;
  }

  switch (event)
  {
  case BTN_1_SHORT_PRESS:
    test_server_connection();
    break;
  default:
    break;
  }
}
