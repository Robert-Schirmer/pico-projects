#include "events.h"

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

void init_screen_manager();

void handle_event_on_screen(EVENT_T event);

void show_boot_screen_sequence(int sequence_number);

void show_boot_screen_msg(char *line1, char *line2, char *line3);

#endif
