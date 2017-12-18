#ifndef _SPI_H
#define _SPI_H
#include "macro.h"
void trxRfSpiInterfaceInit();
rfStatus_t trx8BitRegAccess(uint8 accessType, uint8 addrByte, uint8 *pData, uint16 len);
rfStatus_t trxSpiCmdStrobe(uint8 cmd);
int wait_int(int flag);
void trxRfSpiInterruptInit();
#endif
