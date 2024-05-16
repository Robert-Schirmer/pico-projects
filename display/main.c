#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define SLEEPTIME 25

i2c_inst_t *setup_gpios(void);
ssd1306_t setup_dislay(i2c_inst_t *p_i2c_bus);
void animate_infinitely(ssd1306_t *disp);
void sliding_words_display(ssd1306_t *disp, char *words, int font_size[], int screen_size[]);

int main()
{
    stdio_init_all();

    i2c_inst_t *p_i2c_bus = setup_gpios();
    ssd1306_t disp = setup_dislay(p_i2c_bus);

    char *greeting = "Helllloooooooooooooo, World! How you doing today, I got a long string here for you. Hope you can read all of it while the screen scrolls :D. It's a long one, so I hope you're ready for it! Still going ................. and ...... done nice work";
    for (;;)
    {
        sliding_words_display(&disp, greeting, (int[]){8, 6}, (int[]){32, 128});
    }
}

void sliding_words_display(ssd1306_t *disp, char *string, int font_size[], int screen_size[])
{
    int str_len = strlen(string);
    // Sizes are in pixels height x width, font width should include spacing between characters
    double characters_on_screen = screen_size[1] / font_size[1];
    int characters_off_screen = str_len - characters_on_screen;
    int slide_times = characters_off_screen + 1;

    printf("characters_on_screen: %f\n", characters_on_screen);
    printf("characters_off_screen: %d\n", characters_off_screen);
    printf("slide_times: %d\n", slide_times);

    for (int i = 0; i < slide_times; ++i)
    {
        ssd1306_clear(disp);

        char *word_chopped = string + i;
        ssd1306_draw_string(disp, 1, 1, 1, word_chopped);

        ssd1306_show(disp);
        if ((i == 0 && slide_times > 1) || i == slide_times - 1)
            // Longer sleep for start and end of sliding
            sleep_ms(2000);
        else
            sleep_ms(100);
    }
}

i2c_inst_t *setup_gpios(void)
{
    uint sda_pin = 18;
    uint scl_pin = 19;
    uint baudrate = 400 * 1000;
    i2c_inst_t *i2c_bus = i2c1;

    i2c_init(i2c_bus, baudrate);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    return i2c_bus;
}

ssd1306_t setup_dislay(i2c_inst_t *p_i2c_bus)
{
    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 32, 0x3C, p_i2c_bus);
    ssd1306_clear(&disp);

    return disp;
}

void animate_infinitely(ssd1306_t *p_disp)
{
    const char *words[] = {"SSD1306", "DISPLAY", "DRIVER"};

    ssd1306_clear(p_disp);

    printf("ANIMATION!\n");

    for (;;)
    {
        for (int y = 0; y < 31; ++y)
        {
            ssd1306_draw_line(p_disp, 0, y, 127, y);
            ssd1306_show(p_disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(p_disp);
        }

        for (int y = 0, i = 1; y >= 0; y += i)
        {
            ssd1306_draw_line(p_disp, 0, 31 - y, 127, 31 + y);
            ssd1306_draw_line(p_disp, 0, 31 + y, 127, 31 - y);
            ssd1306_show(p_disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(p_disp);
            if (y == 32)
                i = -1;
        }

        for (int i = 0; i < sizeof(words) / sizeof(char *); ++i)
        {
            ssd1306_draw_string(p_disp, 1, 1, 1, words[i]);
            ssd1306_show(p_disp);
            sleep_ms(800);
            ssd1306_clear(p_disp);
        }

        for (int y = 31; y >= 0; --y)
        {
            ssd1306_draw_line(p_disp, 0, y, 127, y);
            ssd1306_show(p_disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(p_disp);
        }

        ssd1306_show(p_disp);
    }
}
