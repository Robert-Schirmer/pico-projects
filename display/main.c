#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "bootselbtn.h"
#include "flowering.c"
#include "hello.c"

#define SLEEPTIME 500

const uint8_t screen_height = 32;
const uint8_t screen_width = 128;
const uint8_t x_split_point = screen_width / 3 - 1;
const int font_height = 8;
const int font_width = 6;
const int first_line = 0;
const int second_line = font_height + 3;
const int third_line = second_line * 2;

i2c_inst_t *setup_gpios(void);
ssd1306_t setup_dislay(i2c_inst_t *p_i2c_bus);
void draw_frame(ssd1306_t *p_disp, uint8_t image_height, uint8_t image_width, const uint32_t data_array[]);
void do_animation(ssd1306_t *p_disp);

int main()
{
    stdio_init_all();

    i2c_inst_t *p_i2c_bus = setup_gpios();
    ssd1306_t disp = setup_dislay(p_i2c_bus);

    do_animation(&disp);

    for (;;)
    {
        if (get_bootsel_button())
        {
            do_animation(&disp);
        }
        sleep_ms(10);
    }
}

void do_animation(ssd1306_t *p_disp)
{
    ssd1306_clear(p_disp);
    draw_frame(p_disp, HELLO_FRAME_HEIGHT, HELLO_FRAME_WIDTH, hello_data);
    ssd1306_show(p_disp);

    sleep_ms(2000);

    ssd1306_clear(p_disp);
    ssd1306_draw_string(p_disp, x_split_point, screen_height / 2 - 5, 1, "Plantingtosh");

    for (uint8_t i = 0; i < FLOWERING_FRAME_COUNT; i++)
    {
        ssd1306_clear_square(p_disp, 0, 0, x_split_point, screen_height);
        draw_frame(p_disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[i]);
        ssd1306_show(p_disp);
        sleep_ms(SLEEPTIME);
    }

    ssd1306_clear_square(p_disp, x_split_point, 0, screen_width - x_split_point, screen_height);
    ssd1306_draw_string(p_disp, x_split_point, first_line, 1, "Temp  : 25 C");
    ssd1306_draw_string(p_disp, x_split_point, second_line, 1, "Light : 10 Lux");
    ssd1306_draw_string(p_disp, x_split_point, third_line, 1, "Moist : Super");
    ssd1306_show(p_disp);
}

void draw_frame(ssd1306_t *p_disp, uint8_t image_height, uint8_t image_width, const uint32_t data_array[])
{
    int i, x, y;
    for (i = 0; i < image_height * image_width; i++)
    {
        if (i % image_width == 0)
        {
            x = 0;
            y = i / image_width;
        }
        else
        {
            x++;
        }
        if (data_array[i] > 0)
        {
            ssd1306_draw_pixel(p_disp, x, y);
        }
    }
    ssd1306_show(p_disp);
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
    ssd1306_init(&disp, screen_width, screen_height, 0x3C, p_i2c_bus);
    ssd1306_clear(&disp);

    return disp;
}
