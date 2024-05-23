#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "compute.h"
#include "compute_face.h"

#define SLEEPTIME 25

i2c_inst_t *setup_gpios(void);
ssd1306_t setup_dislay(i2c_inst_t *p_i2c_bus);
void draw_image(ssd1306_t *disp, uint8_t image_height, uint8_t image_width, uint8_t image[image_height][image_width]);

int main()
{
    stdio_init_all();

    i2c_inst_t *p_i2c_bus = setup_gpios();
    ssd1306_t disp = setup_dislay(p_i2c_bus);

    for (;;)
    {
        ssd1306_clear(&disp);
        draw_image(&disp, compute_face_height, compute_face_width, compute_face);
        ssd1306_show(&disp);

        sleep_ms(1000);

        ssd1306_clear(&disp);
        draw_image(&disp, compute_height, compute_width, compute);
        ssd1306_show(&disp);

        sleep_ms(1000);
    }
}

void draw_image(ssd1306_t *disp, uint8_t image_height, uint8_t image_width, uint8_t image[image_height][image_width])
{
    int i, j;
    for (i = 0; i < image_height; i++)
    {
        for (j = 0; j < image_width; j++)
        {
            if (image[i][j] == 0)
                ssd1306_draw_pixel(disp, j, i);
        }
    }
    ssd1306_show(disp);
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
