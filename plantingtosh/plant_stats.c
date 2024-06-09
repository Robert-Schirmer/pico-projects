#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "plant_stats.h"
#include "moisture_sensor.h"

#if 0
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

    PLANT_STATS_T *plant_stats = calloc(1, sizeof(PLANT_STATS_T));

    int busy = is_sensor_busy(sensor);
    DEBUG_printf("get_current_stats, sensor busy: %d\n", busy);

    plant_stats->temp = get_sensor_temperature(sensor);
    plant_stats->capacitence = get_sensor_capacitance(sensor);

    return plant_stats;
}

// char *serialize_plant_stats(PLANT_STATS_T *plant_stats)
// {
//     char *temp_prefix = "temp=";
//     char *moisture_prefix = "moist=";
//     char *terminator = "\n";

//     char *serialized = malloc(strlen(temp_prefix) + strlen(plant_stats->temp) +
//                               strlen(moisture_prefix) + strlen(plant_stats->moisture) +
//                               // Plus commas and null terminator
//                               strlen(terminator) + 2 + 1);
//     strcpy(serialized, temp_prefix);
//     strcat(serialized, plant_stats->temp);
//     strcat(serialized, ",");
//     strcat(serialized, ",");
//     strcat(serialized, moisture_prefix);
//     strcat(serialized, plant_stats->moisture);
//     strcat(serialized, terminator);

//     return serialized;
// }
