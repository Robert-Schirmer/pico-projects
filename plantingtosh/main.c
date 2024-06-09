#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/i2c.h"
#include "bootselbtn.h"
#include "btnstate.h"
#include "btnstate.h"
#include "stack_queue.h"
#include "memory_util.h"
#include "ssd1306.h"
#include "flowering.c"
#include "moisture_sensor.h"
#include "plant_stats.h"
#include <math.h>

typedef enum
{
    /**
     * Signals that there are no events in the queue
     */
    NONE,
    BTN_1_SHORT_PRESS,
    BTN_1_LONG_PRESS,
} EVENT_T;

typedef enum
{
    CELCIUS,
    FAHRENHEIT
} TEMP_UNIT_T;

QUEUE stacks_queue;
QUEUE event_queue;

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

void core1_entry()
{
    while (true)
    {
        // If no events, dequeue a stack function
        QUEUE_ITEM stack = dequeue(&stacks_queue);
        if (stack.exists)
        {
            void *execute_func = stack.item;
            ((void (*)(void))execute_func)();
        }

        sleep_ms(1000);
    }
}

void refresh_plant_stats()
{
    printf("refresh_plant_stats\n");
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

    printf("refresh_plant_stats, %s\n", temp_str);
    printf("refresh_plant_stats, %s\n", moisture_str);

    free(stats);
    free(temp_str);
    free(moisture_str);

    printf("refresh_plant_status, done\n");
}

void handle_long_button_press()
{
    printf("handle_long_button_press\n");
    temp_unit = temp_unit == CELCIUS ? FAHRENHEIT : CELCIUS;
    refresh_plant_stats();
}

void handle_short_button_press()
{
    printf("handle_short_button_press\n");
    refresh_plant_stats();
}

void main_tick()
{
    QUEUE_ITEM event = dequeue(&event_queue);
    int e = event.exists ? *((int *)event.item) : NONE;

    switch (e)
    {
    case BTN_1_LONG_PRESS:
        handle_long_button_press();
        break;
    case BTN_1_SHORT_PRESS:
        handle_short_button_press();
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
}

void setup_dislay(ssd1306_t *disp)
{
    disp->external_vcc = false;
    ssd1306_init(disp, screen_width, screen_height, screen_address, i2c0);
    ssd1306_clear(disp);
}

int main()
{
    stdio_init_all();

    queue_init(&stacks_queue, 10);
    queue_init(&event_queue, 5);

    setup_gpios();
    setup_dislay(&disp);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[0]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    printf("Starting core 1\n");
    multicore_reset_core1();
    // Need to sleep for a bit to allow core1 to reset
    sleep_ms(10);
    multicore_launch_core1(core1_entry);
    // Wait for core to start
    sleep_ms(1000);

    ssd1306_clear(&disp);
    draw_frame(&disp, FLOWERING_FRAME_HEIGHT, FLOWERING_FRAME_WIDTH, flowering_data[2]);
    ssd1306_draw_string(&disp, x_split_point, second_line, 1, "Plantingtosh");
    ssd1306_show(&disp);

    int btn_1_long_press = BTN_1_LONG_PRESS;
    int btn_1_short_press = BTN_1_SHORT_PRESS;

    while (true)
    {
        BTN_TICK_STATE_T btn_state = btn_state_tick();

        if (btn_state.released)
        {
            if (LONG_PRESS == btn_state.release_type)
            {
                enqueue(&event_queue, &btn_1_long_press);
            }
            else if (SHORT_PRESS == btn_state.release_type)
            {
                enqueue(&event_queue, &btn_1_short_press);
            }
        }

        main_tick();

        print_free_heap();
        sleep_ms(100);
    }
}
