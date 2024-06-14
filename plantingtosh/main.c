#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "background_tasks.h"
#include "btnstate.h"
#include "flowering.c"
#include "memory_util.h"
#include "moisture_sensor.h"
#include "plant_stats.h"
#include "ssd1306.h"
#include "tcp_client.h"
#include "tasks_watchdog.h"
#include "wifi_cred.h"

#define WIFI_ENABLED true
#define BUTTON_PIN_1 9

typedef enum
{
    /**
     * Signals that there are no events in the queue
     */
    NONE,
    BTN_1_SHORT_PRESS,
    BTN_1_LONG_PRESS,
    BTN_1_LONG_HOLD,
} EVENT_T;

typedef enum
{
    CELCIUS,
    FAHRENHEIT
} TEMP_UNIT_T;

queue_t event_queue;
queue_t background_queue;

ssd1306_t disp;
const uint8_t screen_address = 0x3C;

moisture_sensor_t sensor = {i2c0, SOILMOISTURESENSOR_DEFAULT_ADDR};

const uint screen_height = 32;
const uint screen_width = 128;
const uint x_split_point = screen_width / 4;
const uint font_height = 8;
const uint font_width = 6;
const uint first_line = 0;
const uint second_line = font_height + 3;
const uint third_line = second_line * 2;

int temp_unit = FAHRENHEIT;

int wd_counter __attribute__((section(".unitialized_data")));

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

void log_plant_stats()
{
    PLANT_STATS_T *stats = get_current_stats(&sensor);
    char *serialized = serialize_plant_stats(stats);
    // Serialized already contains a newline
    printf("log_plant_stats, %s", serialized);

    if (WIFI_ENABLED)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        TCP_SERVER_RESPONSE_T *res = send_to_server(serialized);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        free_response(res);
    }

    free(stats);
    free(serialized);
}

void heartbeat_led()
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(50);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

void refresh_show_plant_stats()
{
    DEBUG_printf("refresh_show_plant_stats\n");
    PLANT_STATS_T *stats = get_current_stats(&sensor);

    int temp_tenths = temp_unit == CELCIUS ? stats->temp : (stats->temp * 9) / 5 + 320;

    char *temp_prefix = "Temp  : ";
    // 1 for the decimal point, 1 for space between number and temp_unit, 1 for temp_unit character 1 for null terminator
    char *temp_str = malloc(strlen(temp_prefix) + log10(temp_tenths) + 2 + 1 + 1);
    sprintf(temp_str, "%s%d.%d ", temp_prefix, temp_tenths / 10, temp_tenths % 10);
    if (temp_unit == FAHRENHEIT)
    {
        strcat(temp_str, "F");
    }
    else
    {
        strcat(temp_str, "C");
    }

    char *moisture_prefix = "Moist : ";
    char *moisture_str = malloc(strlen(moisture_prefix) + log10(stats->capacitence) + 1);
    sprintf(moisture_str, "%s%d", moisture_prefix, stats->capacitence);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
    ssd1306_draw_string(&disp, x_split_point, first_line, 1, temp_str);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, moisture_str);
    ssd1306_show(&disp);

    DEBUG_printf("refresh_show_plant_stats, %s\n", temp_str);
    DEBUG_printf("refresh_show_plant_stats, %s\n", moisture_str);

    free(stats);
    free(temp_str);
    free(moisture_str);

    DEBUG_printf("refresh_plant_status, done\n");
}

void core1_entry()
{
    add_background_task("log plant stats", log_plant_stats, 5000);
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

bool screen_on = true;

void toggle_screen_on()
{

    if (screen_on)
    {
        ssd1306_poweroff(&disp);
    }
    else
    {
        ssd1306_poweron(&disp);
    }

    screen_on = !screen_on;
}

void handle_long_button_press()
{
    printf("handle_long_button_press\n");
    if (screen_on)
    {
        // Toggle temp unit (C/F
        temp_unit = temp_unit == CELCIUS ? FAHRENHEIT : CELCIUS;
        refresh_show_plant_stats();
    }
}

void handle_short_button_press()
{
    printf("handle_short_button_press\n");
    if (screen_on)
    {
        refresh_show_plant_stats();
    }
}

void handle_long_hold()
{
    printf("handle_long_hold\n");
    toggle_screen_on();
    if (screen_on)
    {
        refresh_show_plant_stats();
    }
}

void main_tick()
{
    EVENT_T event = NONE;
    queue_try_remove(&event_queue, &event);

    switch (event)
    {
    case BTN_1_LONG_PRESS:
        handle_long_button_press();
        break;
    case BTN_1_SHORT_PRESS:
        handle_short_button_press();
        break;
    case BTN_1_LONG_HOLD:
        handle_long_hold();
        break;
    case NONE:
        // Nothing to do, just keep looping
        break;
    default:
        printf("main_tick, invalid event %d\n", event);
        break;
    }
}

void setup_gpios(void)
{
    uint sda_pin = 16;
    uint scl_pin = 17;
    uint baudrate = 400 * 1000;

    i2c_init(i2c0, baudrate);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1);
}

void setup_dislay(ssd1306_t *disp)
{
    disp->external_vcc = false;
    ssd1306_init(disp, screen_width, screen_height, screen_address, i2c0);
    ssd1306_clear(disp);
}

void connect_wifi(ssd1306_t *disp)
{
    cyw43_arch_enable_sta_mode();

    int wifi_err;

    do
    {
        wifi_err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID_NAME, WIFI_PASSWORD_NAME, CYW43_AUTH_WPA2_AES_PSK, 10000);
        if (wifi_err != 0)
        {
            char *error_code_prefix = "Error code: ";
            char error_code_char[2];
            sprintf(error_code_char, "%d", wifi_err);

            char *error_code_str = malloc(strlen(error_code_prefix) + strlen(error_code_char) + 1);
            strcpy(error_code_str, error_code_prefix);
            strcat(error_code_str, error_code_char);

            ssd1306_clear(disp);
            ssd1306_draw_string(disp, 1, first_line, 1, "Failed to connect");
            ssd1306_draw_string(disp, 1, second_line, 1, error_code_str);
            ssd1306_draw_string(disp, 1, third_line, 1, "Retrying...");
            ssd1306_show(disp);

            free(error_code_str);

            printf("Failed to connect to wifi, error code %d, retrying in 20s\n", wifi_err);
            sleep_ms(20000);
        }
    } while (wifi_err != 0);
}

int main()
{
    stdio_init_all();

    if (watchdog_caused_reboot())
    {
        printf("Watchdog Reboot: %d\n", wd_counter);
        wd_counter += 1;
    }
    else
    {
        printf("Clean boot\n");
        wd_counter = 0;
    }

    queue_init(&event_queue, sizeof(EVENT_T), 10);
    queue_init(&background_queue, sizeof(BACKGROUND_QUEUE_T), 10);

    setup_gpios();
    setup_dislay(&disp);

    if (watchdog_caused_reboot())
    {
        ssd1306_clear(&disp);
        ssd1306_draw_string(&disp, 1, first_line, 1, "Watchdog reboot");
        char *counter_prefix = "counter: ";
        char *counter_str = malloc(strlen(counter_prefix) + log10(wd_counter) + 1 + 1);
        sprintf(counter_str, "counter: %d", wd_counter);
        ssd1306_draw_string(&disp, 1, second_line, 1, counter_str);
        ssd1306_show(&disp);
        sleep_ms(3000);
    }

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[0]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA))
    {
        printf("Failed to initialize arch\n");
        return 1;
    }

    if (WIFI_ENABLED)
    {
        connect_wifi(&disp);
    }

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[1]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    printf("Starting core 1\n");
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(100);
    multicore_launch_core1(core1_entry);
    // Wait for core to start
    sleep_ms(1000);

    init_tasks_watchdog(5000, false, 2);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[2]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    refresh_show_plant_stats();

    int btn_1_long_press = BTN_1_LONG_PRESS;
    int btn_1_short_press = BTN_1_SHORT_PRESS;
    int btn_1_long_hold = BTN_1_LONG_HOLD;

    bool long_hold_handled = false;

    while (true)
    {
        set_task_alive(0);

        BTN_TICK_STATE_T btn_state = btn_state_tick(BUTTON_PIN_1);

        if (btn_state.released)
        {
            if (long_hold_handled)
            {
                // Long hold already handled this release, reset
                long_hold_handled = false;
            }
            else
            {
                if (LONG_PRESS == btn_state.release_type)
                {
                    queue_try_add(&event_queue, &btn_1_long_press);
                }
                else if (SHORT_PRESS == btn_state.release_type)
                {
                    queue_try_add(&event_queue, &btn_1_short_press);
                }
            }
        }
        else if (btn_state.long_hold && !long_hold_handled)
        {
            // Flag the long hold as handled so button release doesn't
            // trigger another handling
            long_hold_handled = true;
            queue_try_add(&event_queue, &btn_1_long_hold);
        }

        main_tick();

        tasks_watchdog_update();
    }
}
