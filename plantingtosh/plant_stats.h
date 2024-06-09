#include "moisture_sensor.h"

#ifndef _inc_plant_stats
#define _inc_plant_stats

typedef struct PLANT_STATS_T_
{
    int temp;
    int capacitence;
} PLANT_STATS_T;

PLANT_STATS_T *get_current_stats(moisture_sensor_t *sensor);

char *serialize_plant_stats(PLANT_STATS_T *plant_stats);

#endif
