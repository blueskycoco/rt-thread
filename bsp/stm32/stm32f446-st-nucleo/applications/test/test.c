#include <stdio.h>
#include <string.h>
#include "lusb0_usb.h"
#include "usb.h"

#include <stdint.h>

void convert2float(int16_t acc[], int16_t gyro[], float accf[], float gyrof[]) {
	int i = 0;
	const double PI = 3.14159265358979323846;

	for (i = 0; i < 3; ++i) {
		accf[i] = acc[i] * 32.0 * 9.8 / 65536.0;
		gyrof[i] = gyro[i] * PI / 180.0 * 4000.0 / 65536.0;
	}
}
int main(int argc, void* argv[])
{
	void *handle;
	int i = 0, j = 0, len;
	uint32_t all_len = 0;
	uint32_t all_rcv = 0;
	int send = 0, rcv_len = 0;
	uint8_t cmd[64] = {0x55, 0x12, 0x33};
	uint8_t rsp[64];
	int16_t acc[6], gyro[6];
	float accf[6], gyrof[6];

	if (argc >= 2)
		send = 1;

	handle = open_usb(0);

	if (handle == NULL) {
		printf("open usb failed\r\n");
		return 0;
	}
	
	memset(cmd, 0x33, 64);
	cmd[62] = '\r';
	cmd[63] = '\n';
	rcv_len = hid_xfer(handle, 0x02, cmd, 64, 1000);
	
	while(1) {
		len = hid_xfer(handle, 0x82, rsp, 49, 1000);
		if (len > 0) {
#if 1
			acc[0] = (int16_t)((rsp[1] << 24) | (rsp[2] << 16) | (rsp[3] << 8) | (rsp[4] << 0));
			acc[1] = (int16_t)((rsp[5] << 24) | (rsp[6] << 16) | (rsp[7] << 8) | (rsp[8] << 0));
			acc[2] = (int16_t)((rsp[9] << 24) | (rsp[10] << 16) | (rsp[11] << 8) | (rsp[12] << 0));
			gyro[0] = (int16_t)((rsp[13] << 24) | (rsp[14] << 16) | (rsp[15] << 8) | (rsp[16] << 0));
			gyro[1] = (int16_t)((rsp[17] << 24) | (rsp[18] << 16) | (rsp[19] << 8) | (rsp[20] << 0));
			gyro[2] = (int16_t)((rsp[21] << 24) | (rsp[22] << 16) | (rsp[23] << 8) | (rsp[24] << 0));
			//printf("icm20602 %10f, %10f, %10f, %10f, %10f, %10f\r\n",
			//		accf[0], accf[1], accf[2],
			//		gyrof[0], gyrof[1], gyrof[2]);
			acc[3] = (int16_t)((rsp[25] << 24) | (rsp[26] << 16) | (rsp[27] << 8) | (rsp[28] << 0));
			acc[4] = (int16_t)((rsp[29] << 24) | (rsp[30] << 16) | (rsp[31] << 8) | (rsp[32] << 0));
			acc[5] = (int16_t)((rsp[33] << 24) | (rsp[34] << 16) | (rsp[35] << 8) | (rsp[36] << 0));
			gyro[3] = (int16_t)((rsp[37] << 24) | (rsp[38] << 16) | (rsp[39] << 8) | (rsp[40] << 0));
			gyro[4] = (int16_t)((rsp[41] << 24) | (rsp[42] << 16) | (rsp[43] << 8) | (rsp[44] << 0));
			gyro[5] = (int16_t)((rsp[45] << 24) | (rsp[46] << 16) | (rsp[47] << 8) | (rsp[48] << 0));
			convert2float(acc, gyro, accf, gyrof);
			convert2float(acc+3, gyro+3, accf+3, gyrof+3);
			printf("icm20602/icm42688 %10f, %10f, %10f, %10f, %10f, %10f,       %10f, %10f, %10f, %10f, %10f, %10f\r\n",
					accf[0], accf[1], accf[2],
					gyrof[0], gyrof[1], gyrof[2],
					accf[3], accf[4], accf[5],
					gyrof[3], gyrof[4], gyrof[5]);
#endif
		} else {
			printf("rcv errno %d\r\n", len);
			break;
		}
	}
	close_usb(handle, 0);

	return 0;
}
