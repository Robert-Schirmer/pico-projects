#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "moisture_sensor.h"
#include "pico/stdlib.h"
#include "plant_stats.h"
#include "utils.h"

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

moisture_sensor_t sensor = {i2c0, SOILMOISTURESENSOR_DEFAULT_ADDR};

PLANT_STATS_T *get_current_stats()
{
    DEBUG_printf("get_current_stats\n");

    // Initialize the sensor by reading
    get_sensor_temperature(&sensor);
    get_sensor_capacitance(&sensor);

    int abort_after_ms = 1000;
    absolute_time_t start_time = get_absolute_time();

    int sensor_busy = 1;

    // Wait for the sensor to not be busy
    do
    {
        sensor_busy = is_sensor_busy(&sensor);
        DEBUG_printf("get_current_stats, sensor busy: %d\n", sensor_busy);

        if (absolute_time_diff_us(start_time, get_absolute_time()) > abort_after_ms * 1000)
        {
            DEBUG_printf("get_current_stats, aborting after %d ms\n", abort_after_ms);
            break;
        }

    } while (sensor_busy == 1);

    PLANT_STATS_T *plant_stats = calloc(1, sizeof(PLANT_STATS_T));

    plant_stats->temp = get_sensor_temperature(&sensor);
    plant_stats->capacitence = get_sensor_capacitance(&sensor);

    return plant_stats;
}

char *serialize_plant_stats(PLANT_STATS_T *plant_stats, char *unique_id)
{
    char *temp_prefix = "temp=";
    char *capacitence_prefix = "capacitence=";
    char *plant_id_prefix = "plant_id=";

    char *terminator = "\n";

    int num_commas = 2;

    char *serialized = malloc(strlen(temp_prefix) + get_number_length(plant_stats->temp) +
                              strlen(capacitence_prefix) + get_number_length(plant_stats->capacitence) +
                              strlen(plant_id_prefix) + strlen(unique_id) +
                              strlen(terminator) + num_commas + 1);

    sprintf(serialized, "%s%d,%s%d,%s%s%s", temp_prefix, plant_stats->temp, capacitence_prefix, plant_stats->capacitence, plant_id_prefix, unique_id, terminator);

    return serialized;
}
