#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "plant_stats.h"

void free_plant_stats(PLANT_STATS_T *plant_stats)
{
    printf("free_plant_stats\n");
    if (plant_stats)
    {
        if (plant_stats->temp)
        {
            free(plant_stats->temp);
        }
        if (plant_stats->light)
        {
            free(plant_stats->light);
        }
        if (plant_stats->moisture)
        {
            free(plant_stats->moisture);
        }
    }
}

PLANT_STATS_T *get_current_stats(void)
{
    printf("get_current_stats\n");
    char *temp_units = "c";
    char *light_units = "lm";
    char *divider = " ";

    int temp = rand() % 20 + 10;
    int light = rand() % 1000 + 1;

    char *moisture_levels[] = {"low", "medium", "high"};
    char *moisture = moisture_levels[rand() % 3];

    PLANT_STATS_T *plant_stats = calloc(1, sizeof(PLANT_STATS_T));

    int char_temp_length = log10(temp);
    int char_light_length = log10(light);

    plant_stats->temp = malloc(char_temp_length + strlen(temp_units) + strlen(divider) + 1);
    plant_stats->light = malloc(char_light_length + strlen(light_units) + strlen(divider) + 1);
    plant_stats->moisture = malloc(strlen(moisture) + 1);

    sprintf(plant_stats->temp, "%d%s%s", temp, divider, temp_units);
    sprintf(plant_stats->light, "%d%s%s", light, divider, light_units);
    sprintf(plant_stats->moisture, "%s", moisture);

    return plant_stats;
}

char *serialize_plant_stats(PLANT_STATS_T *plant_stats)
{
    char *temp_prefix = "temp=";
    char *light_prefix = "light=";
    char *moisture_prefix = "moist=";
    char *terminator = "\n";

    char *serialized = malloc(strlen(temp_prefix) + strlen(plant_stats->temp) +
                              strlen(light_prefix) + strlen(plant_stats->light) +
                              strlen(moisture_prefix) + strlen(plant_stats->moisture) +
                              // Plus commas and null terminator
                              strlen(terminator) + 2 + 1);
    strcpy(serialized, temp_prefix);
    strcat(serialized, plant_stats->temp);
    strcat(serialized, ",");
    strcat(serialized, light_prefix);
    strcat(serialized, plant_stats->light);
    strcat(serialized, ",");
    strcat(serialized, moisture_prefix);
    strcat(serialized, plant_stats->moisture);
    strcat(serialized, terminator);

    return serialized;
}
