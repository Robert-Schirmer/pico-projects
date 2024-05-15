#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

char ssid[] = "coolerfi";
char pass[] = "**";

static void blink_times(int times)
{
    int i;
    for (i = 0; i < times; i++)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);
    }
}

int main()
{
    stdio_init_all();

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA))
    {
        printf("failed to initialise\n");
        return 1;
    }
    printf("initialised\n");
    cyw43_arch_enable_sta_mode();

    blink_times(5);
    int wifi_err = cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000);

    while (true)
    {
        if (wifi_err != 0)
        {
            blink_times(abs(wifi_err));
            printf("sleeping and retrying\n");
            sleep_ms(20000);
            wifi_err = cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000);
        }
        else
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        }
    }
}
