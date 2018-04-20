#include <stdio.h>
unsigned char cmd[] = {0x6c,0xaa,0x06,0x00,0x0e,0x03,0x01};
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
	printf("CRC %x\r\n",CRC_check(cmd,sizeof(cmd)));
	return 0;
}
