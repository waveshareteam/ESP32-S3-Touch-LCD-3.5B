#pragma once

#include "bsp/board.h"

extern bsp_init_status_t bsp_board_status;
esp_err_t bsp_board_init_locks(void);
