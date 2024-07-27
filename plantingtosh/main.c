#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "background_tasks.h"
#include "board_id.h"
#include "btnstate.h"
#include "events.h"
#include "app_macros.h"
#include "app_queues.h"
#include "flowering.c"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "memory_util.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/util/queue.h"
#include "plant_stats.h"
#include "screen_manager.h"
#include "ssd1306.h"
#include "tasks_watchdog.h"
#include "tcp_client.h"
#include "wifi_cred.h"
#include "utils.h"

int wd_counter __attribute__((section(".unitialized_data")));

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

void request_complete_callback(TCP_SERVER_RESPONSE_T *res)
{
    printf("request_complete_callback, request complete, success: %d\n", res->success);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    free_response(res);
}

void log_plant_stats()
{
    PLANT_STATS_T *stats = get_current_stats();
    char *serialized = serialize_plant_stats(stats, get_board_id());
    // Serialized already contains a newline
    printf("log_plant_stats, %s", serialized);

    if (WIFI_ENABLED)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        send_to_server(serialized, request_complete_callback);
    }

    free(stats);
    free(serialized);
}

void heartbeat_led()
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(10);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

void core1_entry()
{
    add_background_task("log plant stats", log_plant_stats, 60000);
    add_background_task("print heap", print_free_heap, 5000);
    add_background_task("heartbeat led", heartbeat_led, 3000);

    while (true)
    {
        set_task_alive(1);

        BACKGROUND_QUEUE_T background_task;

        if (queue_try_remove(&background_queue, &background_task))
        {
            background_task.func();
        }

        enqueue_background_tasks(&background_queue);
    }
}

void setup_gpios(void)
{
    uint baudrate = 400 * 1000;

    i2c_init(i2c0, baudrate);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void connect_wifi()
{
    cyw43_arch_enable_sta_mode();

    int wifi_err;
    int retry_count = 0;
    do
    {
        wifi_err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID_NAME, WIFI_PASSWORD_NAME, CYW43_AUTH_WPA2_AES_PSK, 10000);
        if (wifi_err != 0)
        {
            printf("Failed to connect to wifi, error code %d, retrying in 20s\n", wifi_err);

            char *error_code_prefix = "Error code: ";

            char *error_code_str = malloc(strlen(error_code_prefix) + get_number_length(wifi_err) + 1);
            sprintf(error_code_str, "%s%d", error_code_prefix, wifi_err);

            char *retry_prefix = "Retry count: ";
            char *retry_str = malloc(strlen(retry_prefix) + get_number_length(retry_count) + 1);
            sprintf(retry_str, "%s%d", retry_prefix, retry_count);

            show_boot_screen_msg("Wifi connect", error_code_str, retry_str);

            free(error_code_str);
            free(retry_str);

            sleep_ms(20000);
            retry_count++;
        }
    } while (wifi_err != 0);
}

void main_tick()
{
    EVENT_T event = NONE;
    queue_try_remove(&event_queue, &event);

    handle_event_on_screen(event);
}

int main()
{
    stdio_init_all();
    setup_gpios();

    init_screen_manager();
    show_boot_screen_sequence(0);

    if (watchdog_caused_reboot())
    {
        wd_counter += 1;

        printf("Watchdog Reboot: %d\n", wd_counter);

        char *counter_prefix = "counter: ";
        char *counter_str = malloc(strlen(counter_prefix) + get_number_length(wd_counter) + 1);
        sprintf(counter_str, "%s%d", counter_prefix, wd_counter);
        show_boot_screen_msg("Watchdog reboot", counter_str, "");
        free(counter_str);
    }
    else
    {
        printf("Clean boot\n");
        wd_counter = 0;
    }

    queue_init(&event_queue, sizeof(EVENT_T), 10);
    queue_init(&background_queue, sizeof(BACKGROUND_QUEUE_T), 10);

    BTN_T btn_1;
    BTN_T btn_2;

    init_btn(&btn_1, BUTTON_PIN_1);
    init_btn(&btn_2, BUTTON_PIN_2);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA))
    {
        show_boot_screen_msg("Arch init failed", "", "");
        printf("Failed to initialize arch\n");
        return 1;
    }

    if (WIFI_ENABLED)
    {
        connect_wifi();
    }

    show_boot_screen_sequence(1);

    printf("Starting core 1\n");
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(100);
    multicore_launch_core1(core1_entry);
    // Wait for core to start
    sleep_ms(1000);

    init_tasks_watchdog(5000, false, 2);

    show_boot_screen_sequence(2);

    int btn_1_short_press = BTN_1_SHORT_PRESS;
    int btn_1_long_press = BTN_1_LONG_PRESS;
    int btn_1_long_hold = BTN_1_LONG_HOLD;

    int btn_2_short_press = BTN_2_SHORT_PRESS;
    int btn_2_long_press = BTN_2_LONG_PRESS;
    int btn_2_long_hold = BTN_2_LONG_HOLD;

    bool btn_1_long_hold_handled = false;
    bool btn_2_long_hold_handled = false;

    while (true)
    {
        set_task_alive(0);

        BTN_TICK_STATE_T btn_1_state = btn_state_tick(&btn_1);
        BTN_TICK_STATE_T btn_2_state = btn_state_tick(&btn_2);

        if (btn_1_state.released)
        {
            if (btn_1_long_hold_handled)
            {
                // Long hold already handled this release, reset
                btn_1_long_hold_handled = false;
            }
            else if (SHORT_PRESS == btn_1_state.release_type)
            {
                queue_try_add(&event_queue, &btn_1_short_press);
            }
            else if (LONG_PRESS == btn_1_state.release_type)
            {
                queue_try_add(&event_queue, &btn_1_long_press);
            }
        }
        else if (btn_1_state.long_hold && !btn_1_long_hold_handled)
        {
            // Flag the long hold as handled so button release doesn't
            // trigger another handling
            btn_1_long_hold_handled = true;
            queue_try_add(&event_queue, &btn_1_long_hold);
        }

        if (btn_2_state.released)
        {
            if (btn_2_long_hold_handled)
            {
                // Long hold already handled this release, reset
                btn_2_long_hold_handled = false;
            }
            else if (SHORT_PRESS == btn_2_state.release_type)
            {
                queue_try_add(&event_queue, &btn_2_short_press);
            }
            else if (LONG_PRESS == btn_2_state.release_type)
            {
                queue_try_add(&event_queue, &btn_2_long_press);
            }
        }
        else if (btn_2_state.long_hold && !btn_2_long_hold_handled)
        {
            // Flag the long hold as handled so button release doesn't
            // trigger another handling
            btn_2_long_hold_handled = true;
            queue_try_add(&event_queue, &btn_2_long_hold);
        }

        main_tick();

        tasks_watchdog_update();
    }
}
