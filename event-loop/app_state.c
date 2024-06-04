#include <stdio.h>

typedef struct
{
    int screen;
    int tick_sleep_ms;
} APP_STATE_T;

APP_STATE_T app_state = {0, 100};

int number_of_screens = 2;

int get_app_screen()
{
    return app_state.screen;
}

void next_app_screen()
{
    if (app_state.screen == number_of_screens - 1)
    {
        app_state.screen = 0;
    }
    else
    {
        app_state.screen++;
    }
    printf("next_app_screen, %d\n", app_state.screen);
}

int get_tick_sleep_ms()
{
    return app_state.tick_sleep_ms;
}

void set_awake_tick_sleep_ms()
{
    app_state.tick_sleep_ms = 100;
}

void set_sleep_tick_sleep_ms()
{
    app_state.tick_sleep_ms = 2000;
}
