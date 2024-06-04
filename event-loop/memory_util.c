#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include <malloc.h>

uint32_t get_total_heap(void)
{
    extern char __StackLimit, __bss_end__;

    return &__StackLimit - &__bss_end__;
}

uint32_t get_free_heap(void)
{
    struct mallinfo m = mallinfo();

    return get_total_heap() - m.uordblks;
}

absolute_time_t last_free_heap_print;
int last_free_heap = -1;
uint print_free_heap_interval_s = 5;

void print_free_heap(void)
{
    if (absolute_time_diff_us(last_free_heap_print, get_absolute_time()) < 1000000 * print_free_heap_interval_s)
    {
        return;
    }
    int free_heap = get_free_heap();
    int diff = -1 == last_free_heap ? 0 : last_free_heap - free_heap;
    printf("Free heap: %d, Diff: %d\n", free_heap, diff);

    last_free_heap_print = get_absolute_time();
    last_free_heap = free_heap;
}
