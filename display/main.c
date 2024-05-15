#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define SLEEPTIME 25
#define IC2_BUS i2c1

void setup_gpios(void);
void animation(void);

int main()
{
    stdio_init_all();

    printf("configuring pins...\n");
    setup_gpios();

    printf("jumping to animation...\n");

    animation();
}

void setup_gpios(void)
{
    const uint sda_pin = 18;
    const uint scl_pin = 19;

    i2c_init(IC2_BUS, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

void animation(void)
{
    const char *words[] = {"SSD1306", "DISPLAY", "DRIVER"};

    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 32, 0x3C, IC2_BUS);
    ssd1306_clear(&disp);

    printf("ANIMATION!\n");

    for (;;)
    {
        for (int y = 0; y < 31; ++y)
        {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        for (int y = 0, i = 1; y >= 0; y += i)
        {
            ssd1306_draw_line(&disp, 0, 31 - y, 127, 31 + y);
            ssd1306_draw_line(&disp, 0, 31 + y, 127, 31 - y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
            if (y == 32)
                i = -1;
        }

        for (int i = 0; i < sizeof(words) / sizeof(char *); ++i)
        {
            ssd1306_draw_string(&disp, 1, 1, 1, words[i]);
            ssd1306_show(&disp);
            sleep_ms(800);
            ssd1306_clear(&disp);
        }

        for (int y = 31; y >= 0; --y)
        {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        ssd1306_show(&disp);
    }
}
