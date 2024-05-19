#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi_cred.h"

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
        printf("Failed to initialise\n");
        return 1;
    }
    printf("Initialised\n");
    cyw43_arch_enable_sta_mode();

    blink_times(5);

    int wifi_err;

    do
    {
        printf("Connecting to " WIFI_SSID_NAME "\n");
        wifi_err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID_NAME, WIFI_PASSWORD_NAME, CYW43_AUTH_WPA2_AES_PSK, 10000);
        if (wifi_err != 0)
        {
            char *error_code_prefix = "Error code: ";
            char error_code_char[2];
            sprintf(error_code_char, "%d", wifi_err);

            char *error_code_str = malloc(strlen(error_code_prefix) + strlen(error_code_char) + 1);
            strcpy(error_code_str, error_code_prefix);
            strcat(error_code_str, error_code_char);

            free(error_code_str);

            printf("Failed to connect to wifi, error code %d, retrying in 20s\n", wifi_err);
            sleep_ms(20000);
        }
    } while (wifi_err != 0);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    printf("Connected to " WIFI_SSID_NAME "\n");

    for(;;)
    {
    }
}
