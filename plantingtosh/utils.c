#include <stdio.h>
#include "utils.h"

int get_number_length(int number)
{
  // If number is less than zero, add one to the length for the negative sign
  int length = number < 0 ? 1 : 0;
  int remainder = number;
  while (remainder != 0)
  {
    remainder /= 10;
    length++;
  }
  return length;
}
