#include <stdio.h>
unsigned char cmd[] = {0xad,0xac,0x0,0x1f,0x0,0x80,0x1,0x0,0xa1,0x18,0x1,0x21,0x23,0x45,0x0,0x14,0x12,0x3,0x3,0x9,0x32,0x0,0x6,0x41,0x42,0x43,0x44,0x45,0x46};
unsigned int CRC_check(unsigned char *Data,unsigned short Data_length)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned int CRC=0xFFFF;
	while(Data_length)
	{
		CRC=Data[Data_index]^CRC;
		for(times=0;times<8;times++)
		{
			mid=CRC;
			CRC=CRC>>1;
			if(mid & 0x0001)
			{
				CRC=CRC^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC;
}

int main(int argc, void *argv[])
{
	printf("CRC %x\r\n",CRC_check(cmd+2,sizeof(cmd)-2));
	return 0;
}
