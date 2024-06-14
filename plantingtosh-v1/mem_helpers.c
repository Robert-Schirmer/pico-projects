#include <malloc.h>
#include <stdint.h>

uint32_t get_total_heap(void) {
   extern char __StackLimit, __bss_end__;
   
   return &__StackLimit  - &__bss_end__;
}

uint32_t get_free_heap(void) {
   struct mallinfo m = mallinfo();

   return get_total_heap() - m.uordblks;
}
