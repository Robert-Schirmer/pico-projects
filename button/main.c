#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 6

int main()
{
    stdio_init_all();

    cyw43_arch_init();

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1);

    gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_2);

    while (true)
    {
        if (!gpio_get(BUTTON_PIN_1))
        {
            printf("Button 1 pressed\n");
        }

        if (!gpio_get(BUTTON_PIN_2))
        {
            printf("Button 2 pressed\n");
        }

        sleep_ms(10);
    }
}
