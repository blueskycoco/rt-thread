#ifndef __CMD_CRC_H__
#define __CMD_CRC_H__

#include <rthw.h>
#include <rtthread.h>
uint32_t heart_cmd_crc(uint8_t* buf, uint32_t len);
void ByteToHexStr(uint32_t source, char* dest);
#endif
