#include <stdio.h>
#include <stdlib.h>
unsigned int CRC_check_file(unsigned char *file)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned char *Data = NULL;
	unsigned short Data_length=0,Data_length1;
	unsigned int CRC1=0xFFFF;

	int fd = open(file, 0, 0);
	if (fd < 0)
	{
		printf("CRC check file ,open %s failed\n");
		return 0;
	}
	Data = (unsigned char *)malloc(1024*sizeof(unsigned char));
	if (Data == NULL) {
		printf("CRC check file , malloc failed\r\n");
		return 0;
	}

	do {
		Data_length1 = Data_length = read(fd, Data, 1024);
//		printf("Data length %d\r\n", Data_length);
		if (Data_length > 0) {
			Data_index=0;
			while (Data_length) {
				CRC1=Data[Data_index]^CRC1;
				for(times=0;times<8;times++)
				{
					mid=CRC1;
					CRC1=CRC1>>1;
					if(mid & 0x0001)
					{
						CRC1=CRC1^0xA001;
					}
				}
				Data_index++;
				Data_length--;
			}
		}
		else
		{
			printf("read %s failed %d\r\n", file,CRC1);
		}

	}while(Data_length1 == 1024);
	close(fd);
	free(Data);
	printf("CRC check file crc is %04x\r\n", CRC1);
	return CRC1;
}

int main(int argc, void *argv[])
{
	CRC_check_file(argv[1]);
	return 0;
}
