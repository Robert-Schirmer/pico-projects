#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "tcp_client.h"
#include "wifi_cred.h"
#include "bootselbtn.h"
#include "flowering.c"
#include "plant_stats.h"
#include "mem_helpers.h"

const uint screen_height = 32;
const uint screen_width = 128;
const uint x_split_point = screen_width / 4;
const uint font_height = 8;
const uint font_width = 6;
const uint first_line = 0;
const uint second_line = font_height + 3;
const uint third_line = second_line * 2;

i2c_inst_t *setup_gpios(void);
ssd1306_t setup_dislay(i2c_inst_t *p_i2c_bus);
void clear_draw_string_show(ssd1306_t *p_disp, char *first_line_string, char *second_line_string, char *third_line_string);
void connect_wifi(ssd1306_t *p_disp);
void draw_frame(ssd1306_t *p_disp, uint image_height, uint image_width, const uint32_t data_array[]);
void show_current_stats(ssd1306_t *p_disp, PLANT_STATS_T *plant_stats);
void draw_plant_response_string(ssd1306_t *p_disp, char *word);

int main()
{
    srand(time(NULL));

    stdio_init_all();

    i2c_inst_t *p_i2c_bus = setup_gpios();
    ssd1306_t disp = setup_dislay(p_i2c_bus);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[0]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);
    printf("initializing...\n");

    sleep_ms(1000);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA))
    {
        clear_draw_string_show(&disp, "Failed to initialize", NULL, NULL);
        printf("Failed to initialize");
        return 1;
    }

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[1]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    sleep_ms(1000);

    connect_wifi(&disp);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[2]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    sleep_ms(1000);

    uint screen = 0;
    TCP_SERVER_RESPONSE_T *res;
    absolute_time_t scroll_again_time = get_absolute_time();
    absolute_time_t stat_refresh_time = get_absolute_time();
    absolute_time_t mem_readout_time = get_absolute_time();
    PLANT_STATS_T *plant_stats = get_current_stats();

    while (true)
    {
        if (get_bootsel_button())
        {
            if (0 == screen)
            {
                screen = 1;

                stat_refresh_time = get_absolute_time();

                ssd1306_clear(&disp);
                draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[2]);
                draw_plant_response_string(&disp, "I'm feeling...");
                ssd1306_show(&disp);

                char *msg_body = serialize_plant_stats(plant_stats);
                res = send_to_server(msg_body);
                free(msg_body);
            }
            else
            {
                screen = 0;

                scroll_again_time = get_absolute_time();
                free_response(res);
            }
        }

        if (1 == screen && absolute_time_diff_us(scroll_again_time, get_absolute_time()) > 0)
        {
            sliding_words_display(&disp, res->data, font_width, screen_width - x_split_point, &draw_plant_response_string);
            scroll_again_time = make_timeout_time_ms(5000);
        }
        else if (0 == screen && absolute_time_diff_us(stat_refresh_time, get_absolute_time()) > 0)
        {
            free_plant_stats(plant_stats);
            plant_stats = get_current_stats();
            show_current_stats(&disp, plant_stats);
            stat_refresh_time = make_timeout_time_ms(10000);
        }

        if (absolute_time_diff_us(mem_readout_time, get_absolute_time()) > 0)
        {
            printf("free heap: %d\n", get_free_heap());
            mem_readout_time = make_timeout_time_ms(10000);
        }

        sleep_ms(10);
    }
}

void draw_plant_response_string(ssd1306_t *p_disp, char *word)
{
    ssd1306_clear_square(p_disp, x_split_point, 0, screen_width - x_split_point, screen_height);
    ssd1306_draw_string(p_disp, x_split_point, second_line, 1, word);
    ssd1306_show(p_disp);
}

void show_current_stats(ssd1306_t *p_disp, PLANT_STATS_T *plant_stats)
{
    ssd1306_clear(p_disp);

    draw_frame(p_disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);

    char *temp_prefix = "Temp  : ";

    char *temp_str = malloc(strlen(temp_prefix) + strlen(plant_stats->temp) + 1);
    strcpy(temp_str, temp_prefix);
    strcat(temp_str, plant_stats->temp);

    char *light_prefix = "Light : ";
    char *light_str = malloc(strlen(light_prefix) + strlen(plant_stats->light) + 1);
    strcpy(light_str, light_prefix);
    strcat(light_str, plant_stats->light);

    char *moisture_prefix = "Moist : ";
    char *moisture_str = malloc(strlen(moisture_prefix) + strlen(plant_stats->moisture) + 1);
    strcpy(moisture_str, moisture_prefix);
    strcat(moisture_str, plant_stats->moisture);

    ssd1306_draw_string(p_disp, x_split_point, first_line, 1, temp_str);
    ssd1306_draw_string(p_disp, x_split_point, second_line, 1, light_str);
    ssd1306_draw_string(p_disp, x_split_point, third_line, 1, moisture_str);
    ssd1306_show(p_disp);

    free(temp_str);
    free(light_str);
    free(moisture_str);

    uint jump_times = 1;

    for (uint i = 0; i < jump_times; i++)
    {
        sleep_ms(500);
        ssd1306_clear_square(p_disp, 0, 0, x_split_point, screen_height);
        draw_frame(p_disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[4]);
        ssd1306_show(p_disp);

        sleep_ms(500);
        ssd1306_clear_square(p_disp, 0, 0, x_split_point, screen_height);
        draw_frame(p_disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[3]);
        ssd1306_show(p_disp);
    }
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

            clear_draw_string_show(disp, "Failed to connect", error_code_str, "Retrying...");
            free(error_code_str);

            printf("Failed to connect to wifi, error code %d, retrying in 20s\n", wifi_err);
            sleep_ms(20000);
        }
    } while (wifi_err != 0);
}

void draw_frame(ssd1306_t *p_disp, uint image_height, uint image_width, const uint32_t data_array[])
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

void clear_draw_string_show(ssd1306_t *disp, char *first_line_string, char *second_line_string, char *third_line_string)
{
    ssd1306_clear(disp);
    ssd1306_draw_string(disp, 1, first_line, 1, first_line_string);
    ssd1306_draw_string(disp, 1, second_line, 1, second_line_string);
    ssd1306_draw_string(disp, 1, third_line, 1, third_line_string);
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
