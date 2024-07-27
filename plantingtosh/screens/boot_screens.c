#include <stdlib.h>
#include <stdio.h>
#include "plant_stats_screens.h"
#include "boot_screens.h"
#include "../screen_manager.h"
#include "screen_ids.h"

void boot_screen_handle_event(EVENT_T event);
void boot_screen_exit_entry();
void boot_screen_entry();

Screen boot_screen;
Screen boot_screen_exit;

ScreenGroup boot_screen_group = {
    .id = BOOT_SCREEN_GOUP_ID,
    .first_screen = &boot_screen,
    .next_screen_group = &plant_stats_screen_group,
    .init = NULL,
};

Screen boot_screen = {
    .id = BOOT_SCREEN_ID,
    .entry = boot_screen_entry,
    .handle_event = boot_screen_handle_event,
    .next_screen = &boot_screen_exit,
};

// Fake screen that forwards to the next screen group
Screen boot_screen_exit = {
    .id = BOOT_SCREEN_EXIT_ID,
    .entry = boot_screen_exit_entry,
    .handle_event = NULL,
    .next_screen = NULL,
};

void boot_screen_handle_event(EVENT_T event)
{
  // Any button press will skip the boot screen
  if (event != NONE)
  {
    next_screen_group();
  }
}

void boot_screen_exit_entry()
{
  next_screen_group();
}

void boot_screen_entry() {
    printf("boot_screen_entry\n");
}
