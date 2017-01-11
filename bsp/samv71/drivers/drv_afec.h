#ifndef __DRV_AFEC_H
#define __DRV_AFEC_H

#include "rt_board.h"
bool AFEC_init(int AFEC_ID, int CH_ID);
bool AFEC_get_data(int AFEC_ID, int CH_ID, uint32_t *data);
#endif

