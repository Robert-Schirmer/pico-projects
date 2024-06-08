#ifndef _inc_plant_stats
#define _inc_plant_stats

typedef struct PLANT_STATS_T_
{
    char *temp;
    char *light;
    char *moisture;
} PLANT_STATS_T;

void free_plant_stats(PLANT_STATS_T *plant_stats);

PLANT_STATS_T *get_current_stats(void);

char *serialize_plant_stats(PLANT_STATS_T *plant_stats);

#endif
