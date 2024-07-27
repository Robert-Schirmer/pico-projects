#include "../screen_manager.h"

#ifndef SLEEP_SCREENS_H
#define SLEEP_SCREENS_H

extern ScreenGroup sleep_screen_group;
extern Screen sleep_screen;

bool handle_sleep_check(EVENT_T event);

#endif
