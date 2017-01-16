#ifndef __DRV_QSPI_H
#define __DRV_QSPI_H

#include <rtdevice.h>

#include "rt_board.h"

//#define SPI_USE_DMA

struct same70_qspi_bus
{
    struct rt_spi_bus parent;    
};

/* public function list */
void rt_hw_spi_init(void);
#endif 
