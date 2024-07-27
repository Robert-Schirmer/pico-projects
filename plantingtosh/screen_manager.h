#include "events.h"
#include "ssd1306.h"

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

typedef struct Screen
{
  int id;
  void (*entry)();
  void (*handle_event)(EVENT_T event);
  struct Screen *next_screen;
} Screen;

typedef struct ScreenGroup
{
  int id;
  struct Screen *first_screen;
  struct ScreenGroup *next_screen_group;
  void (*init)(ssd1306_t *disp);
} ScreenGroup;

void init_screen_manager();

void handle_event_on_screen(EVENT_T event);

void show_boot_screen_sequence(int sequence_number);

void show_boot_screen_msg(char *line1, char *line2, char *line3);

void next_screen_group();

void jump_to_screen_group(ScreenGroup *screen_group);

int is_current_screen(Screen *screen);

#endif
