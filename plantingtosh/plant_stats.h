#include "moisture_sensor.h"

#ifndef _inc_plant_stats
#define _inc_plant_stats

typedef struct PLANT_STATS_T_
{
    int temp;
    int capacitence;
} PLANT_STATS_T;

typedef enum
{
    CELCIUS,
    FAHRENHEIT
} TEMP_UNIT_T;

/**
 * Get the current stats from the sensor, will block until the sensor is ready
 */
PLANT_STATS_T *get_current_stats();

/**
 * Serialize the plant stats into a string with a unique identifier for the plant
 */
char *serialize_plant_stats(PLANT_STATS_T *plant_stats, char *unique_id);

#endif
