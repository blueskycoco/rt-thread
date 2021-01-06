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
	int16_t acc[3], gyro[3];
	float accf[3], gyrof[3];

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
		len = hid_xfer(handle, 0x82, rsp, 13, 1000);
		if (len > 0) {
#if 1
			acc[0] = (rsp[1] << 8) | rsp[2];
			acc[1] = (rsp[3] << 8) | rsp[4];
			acc[2] = (rsp[5] << 8) | rsp[6];
			gyro[0] = (rsp[7] << 8) | rsp[8];
			gyro[1] = (rsp[9] << 8) | rsp[10];
			gyro[2] = (rsp[11] << 8) | rsp[12];
			convert2float(acc, gyro, accf, gyrof);
			printf("acc[%d] %10f %10f %10f, gyro %10f %10f %10f\r\n",
					rsp[0], accf[0], accf[1], accf[2],
					gyrof[0], gyrof[1], gyrof[2]);
#endif
		} else {
			printf("rcv errno %d\r\n", len);
			break;
		}
	}
	close_usb(handle, 0);

	return 0;
}
