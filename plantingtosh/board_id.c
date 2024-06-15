#include "board_id.h"
#include "pico/unique_id.h"

#define BOARD_ID_LEN 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1

static char board_id[BOARD_ID_LEN];

char *get_board_id()
{
  pico_get_unique_board_id_string(board_id, BOARD_ID_LEN);
  return board_id;
}
