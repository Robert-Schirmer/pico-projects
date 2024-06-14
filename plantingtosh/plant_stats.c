#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "plant_stats.h"
#include "moisture_sensor.h"

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

PLANT_STATS_T *get_current_stats(moisture_sensor_t *sensor)
{
    DEBUG_printf("get_current_stats\n");

    // Initialize the sensor by reading
    get_sensor_temperature(sensor);
    get_sensor_capacitance(sensor);

    int abort_after_ms = 1000;
    absolute_time_t start_time = get_absolute_time();

    int sensor_busy = 1;

    // Wait for the sensor to not be busy
    do
    {
        sensor_busy = is_sensor_busy(sensor);
        DEBUG_printf("get_current_stats, sensor busy: %d\n", sensor_busy);

        if (absolute_time_diff_us(start_time, get_absolute_time()) > abort_after_ms * 1000)
        {
            DEBUG_printf("get_current_stats, aborting after %d ms\n", abort_after_ms);
            break;
        }

    } while (sensor_busy == 1);

    PLANT_STATS_T *plant_stats = calloc(1, sizeof(PLANT_STATS_T));

    plant_stats->temp = get_sensor_temperature(sensor);
    plant_stats->capacitence = get_sensor_capacitance(sensor);

    return plant_stats;
}

char *serialize_plant_stats(PLANT_STATS_T *plant_stats)
{
    char *temp_prefix = "temp=";
    char *capacitence_prefix = "capacitence=";
    char *terminator = "\n";

    // Plus 2 if negative for the minus sign and the digit less than 10 that drops to a decimal place
    int temp_str_length = log10(abs(plant_stats->temp)) + (plant_stats->temp >= 0 ? 1 : 2);
    int capacitence_str_length = log10(abs(plant_stats->capacitence)) + (plant_stats->capacitence >= 0 ? 1 : 2);

    DEBUG_printf("serialize_plant_stats, temp_str_length: %d, capacitence_str_length: %d\n", temp_str_length, capacitence_str_length);

    char *serialized = malloc(strlen(temp_prefix) + temp_str_length +
                              strlen(capacitence_prefix) + capacitence_str_length +
                              // Plus commas and null terminator
                              strlen(terminator) + 1 + 1);
    sprintf(serialized, "%s%d,%s%d%s", temp_prefix, plant_stats->temp, capacitence_prefix, plant_stats->capacitence, terminator);

    return serialized;
}
