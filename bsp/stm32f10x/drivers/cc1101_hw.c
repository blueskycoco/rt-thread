#include "cc1101.h"
#include "led.h"
#define 	WRITE_BURST     		0x40						//����д��
#define 	READ_SINGLE     		0x80						//��
#define 	READ_BURST      		0xC0						//������
#define 	BYTES_IN_RXFIFO     	0x7F  						//���ջ���������Ч�ֽ���
#define 	CRC_OK              		0x80 						//CRCУ��ͨ��λ��־
#define 	TYPE_BURST 			0
#define 	TYPE_REG 				1
#define 	TYPE_STROBE_STATUS 2
uint8_t PaTabel[8] = {0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60};
//*****************************************************************************************
// CC1100 STROBE, CONTROL AND STATUS REGSITER
#define CCxxx0_IOCFG2       0x00        // GDO2 output pin configuration
#define CCxxx0_IOCFG1       0x01        // GDO1 output pin configuration
#define CCxxx0_IOCFG0       0x02        // GDO0 output pin configuration
#define CCxxx0_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CCxxx0_SYNC1        0x04        // Sync word, high uint8_t
#define CCxxx0_SYNC0        0x05        // Sync word, low uint8_t
#define CCxxx0_PKTLEN       0x06        // Packet length
#define CCxxx0_PKTCTRL1     0x07        // Packet automation control
#define CCxxx0_PKTCTRL0     0x08        // Packet automation control
#define CCxxx0_ADDR         0x09        // Device address
#define CCxxx0_CHANNR       0x0A        // Channel number
#define CCxxx0_FSCTRL1      0x0B        // Frequency synthesizer control
#define CCxxx0_FSCTRL0      0x0C        // Frequency synthesizer control
#define CCxxx0_FREQ2        0x0D        // Frequency control word, high uint8_t
#define CCxxx0_FREQ1        0x0E        // Frequency control word, middle uint8_t
#define CCxxx0_FREQ0        0x0F        // Frequency control word, low uint8_t
#define CCxxx0_MDMCFG4      0x10        // Modem configuration
#define CCxxx0_MDMCFG3      0x11        // Modem configuration
#define CCxxx0_MDMCFG2      0x12        // Modem configuration
#define CCxxx0_MDMCFG1      0x13        // Modem configuration
#define CCxxx0_MDMCFG0      0x14        // Modem configuration
#define CCxxx0_DEVIATN      0x15        // Modem deviation setting
#define CCxxx0_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CCxxx0_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CCxxx0_BSCFG        0x1A        // Bit Synchronization configuration
#define CCxxx0_AGCCTRL2     0x1B        // AGC control
#define CCxxx0_AGCCTRL1     0x1C        // AGC control
#define CCxxx0_AGCCTRL0     0x1D        // AGC control
#define CCxxx0_WOREVT1      0x1E        // High uint8_t Event 0 timeout
#define CCxxx0_WOREVT0      0x1F        // Low uint8_t Event 0 timeout
#define CCxxx0_WORCTRL      0x20        // Wake On Radio control
#define CCxxx0_FREND1       0x21        // Front end RX configuration
#define CCxxx0_FREND0       0x22        // Front end TX configuration
#define CCxxx0_FSCAL3       0x23        // Frequency synthesizer calibration
#define CCxxx0_FSCAL2       0x24        // Frequency synthesizer calibration
#define CCxxx0_FSCAL1       0x25        // Frequency synthesizer calibration
#define CCxxx0_FSCAL0       0x26        // Frequency synthesizer calibration
#define CCxxx0_RCCTRL1      0x27        // RC oscillator configuration
#define CCxxx0_RCCTRL0      0x28        // RC oscillator configuration
#define CCxxx0_FSTEST       0x29        // Frequency synthesizer calibration control
#define CCxxx0_PTEST        0x2A        // Production test
#define CCxxx0_AGCTEST      0x2B        // AGC test
#define CCxxx0_TEST2        0x2C        // Various test settings
#define CCxxx0_TEST1        0x2D        // Various test settings
#define CCxxx0_TEST0        0x2E        // Various test settings

// Strobe commands
#define CCxxx0_SRES         0x30        // Reset chip.
#define CCxxx0_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
// If in RX/TX: Go to a wait state where only the synthesizer is
// running (for quick RX / TX turnaround).
#define CCxxx0_SXOFF        0x32        // Turn off crystal oscillator.
#define CCxxx0_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
// (enables quick start).
#define CCxxx0_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
// MCSM0.FS_AUTOCAL=1.
#define CCxxx0_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
// MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
// Only go to TX if channel is clear.
#define CCxxx0_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
// Wake-On-Radio mode if applicable.
#define CCxxx0_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CCxxx0_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CCxxx0_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CCxxx0_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CCxxx0_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CCxxx0_SWORRST      0x3C        // Reset real time clock.
#define CCxxx0_SNOP         0x3D        // No operation. May be used to pad strobe commands to two
// uint8_ts for simpler software.

#define CCxxx0_PARTNUM      0x30
#define CCxxx0_VERSION      0x31
#define CCxxx0_FREQEST      0x32
#define CCxxx0_LQI          0x33
#define CCxxx0_RSSI         0x34
#define CCxxx0_MARCSTATE    0x35
#define CCxxx0_WORTIME1     0x36
#define CCxxx0_WORTIME0     0x37
#define CCxxx0_PKTSTATUS    0x38
#define CCxxx0_VCO_VC_DAC   0x39
#define CCxxx0_TXBYTES      0x3A
#define CCxxx0_RXBYTES      0x3B

#define CCxxx0_PATABLE      0x3E
#define CCxxx0_TXFIFO       0x3F
#define CCxxx0_RXFIFO       0x3F

typedef struct S_RF_SETTINGS
{
    uint8_t FSCTRL2;   //���Ѽӵ�
    uint8_t FSCTRL1;   // Frequency synthesizer control.
    uint8_t FSCTRL0;   // Frequency synthesizer control.
    uint8_t FREQ2;     // Frequency control word, high uint8_t.
    uint8_t FREQ1;     // Frequency control word, middle uint8_t.
    uint8_t FREQ0;     // Frequency control word, low uint8_t.
    uint8_t MDMCFG4;   // Modem configuration.
    uint8_t MDMCFG3;   // Modem configuration.
    uint8_t MDMCFG2;   // Modem configuration.
    uint8_t MDMCFG1;   // Modem configuration.
    uint8_t MDMCFG0;   // Modem configuration.
    uint8_t CHANNR;    // Channel number.
    uint8_t DEVIATN;   // Modem deviation setting (when FSK modulation is enabled).
    uint8_t FREND1;    // Front end RX configuration.
    uint8_t FREND0;    // Front end RX configuration.
    uint8_t MCSM0;     // Main Radio Control State Machine configuration.
    uint8_t FOCCFG;    // Frequency Offset Compensation Configuration.
    uint8_t BSCFG;     // Bit synchronization Configuration.
    uint8_t AGCCTRL2;  // AGC control.
    uint8_t AGCCTRL1;  // AGC control.
    uint8_t AGCCTRL0;  // AGC control.
    uint8_t FSCAL3;    // Frequency synthesizer calibration.
    uint8_t FSCAL2;    // Frequency synthesizer calibration.
    uint8_t FSCAL1;    // Frequency synthesizer calibration.
    uint8_t FSCAL0;    // Frequency synthesizer calibration.
    uint8_t FSTEST;    // Frequency synthesizer calibration control
    uint8_t TEST2;     // Various test settings.
    uint8_t TEST1;     // Various test settings.
    uint8_t TEST0;     // Various test settings.
    uint8_t IOCFG2;    // GDO2 output pin configuration
    uint8_t IOCFG0;    // GDO0 output pin configuration
    uint8_t PKTCTRL1;  // Packet automation control.
    uint8_t PKTCTRL0;  // Packet automation control.
    uint8_t ADDR;      // Device address.
    uint8_t PKTLEN;    // Packet length.
    uint8_t SYNC1;    // Packet length.
    uint8_t SYNC0;    // Packet length.
} RF_SETTINGS;
#define mdmcfg2 0x03 //non MANCHESTER_EN,0x0b
#define pktctrl1 0x04 //use crc check , 0x00 no crc
#define pktctrl0 0x05 //sync check,use crc check ,0x01 just sync check,0x00 no sync,preamble check"
#define deviatn 0x43 //bigest ,0x73
RF_SETTINGS rfSettings = 
{
    0x00,
    0x0a,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x10,   // FREQ2     Frequency control word, high byte.
    0xB0,   // FREQ1     Frequency control word, middle byte.
    0x92,   // FREQ0     Frequency control word, low byte.
    0xe8,   // MDMCFG4   Modem configuration.//old 0x5b
    0x83,   // MDMCFG3   Modem configuration.//old f8
    0x0b,   // MDMCFG2   Modem configuration.//old 03
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.

    0x00,   // CHANNR    Channel number.
    0x43,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0xb6,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end RX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x1d,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x1C,   // BSCFG     Bit synchronization Configuration.
    0xc7,   // AGCCTRL2  AGC control.
    0x00,   // AGCCTRL1  AGC control.
    0xb0,   // AGCCTRL0  AGC control.

    0xE9,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x1f,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x81,   // TEST2     Various test settings.
    0x35,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.
    0x29,   // IOCFG2    GDO2 output pin configuration.
    #if CC1101_RCV
0x06,
	#else
0x06,
	#endif
    //0x06,   // IOCFG0D   GDO0 output pin configuration. Refer to SmartRF?Studio User Manual for detailed pseudo register explanation. //old 0x06

    0x04,   // PKTCTRL1  Packet automation control.
    0x05,   // PKTCTRL0  Packet automation control.//old 0x05
    0x00,   // ADDR      Device address.
    #if CC1101_RCV
    0xff,    // PKTLEN    Packet length.
    #else
	0x0c,
	#endif
	0xff,
	0xfe
};

void reset_cc1101()
{
    uint8_t val=CCxxx0_SRES;
    reset_cs();	
    cs(0);
    while(miso());
    spi_send_rcv(val);
    while(miso());
    cs(1);
    return;
}

void write_cc1101(uint8_t addr,uint8_t* buf,uint8_t len,uint8_t type)
{
    uint8_t cmd;
    int i;
    cs(0);
    while(miso());
    if(type==TYPE_BURST)
    {/*write brust */
        cmd = addr | WRITE_BURST;
        spi_send_rcv(cmd);
        for(i=0;i<len;i++)
            spi_send_rcv(buf[i]);
    }
    else if(type==TYPE_REG)
    {/*write reg */
        spi_send_rcv(addr);
        spi_send_rcv(buf[0]);
    }
    else/* strobe */
        spi_send_rcv(addr);
    cs(1);
}

uint8_t read_cc1101(uint8_t addr,uint8_t *buf,uint8_t len,uint8_t type)
{
    uint8_t cmd,data=0,i;
    cs(0);
    while(miso());
    if(type==TYPE_BURST)
    {
        cmd = addr | READ_BURST;
        spi_send_rcv(cmd);
        for(i=0;i<len;i++)
            buf[i]=spi_send_rcv(0);
    }
    else if(type==TYPE_REG)
    {
        cmd = addr | READ_BURST;
        spi_send_rcv(cmd);
        return spi_send_rcv(0);
    }
    else
    {
        cmd = addr | READ_SINGLE;
        spi_send_rcv(cmd);
        return spi_send_rcv(0);
    }
    cs(1);
    return 0;
}

void init_rf(void) 
{

    write_cc1101(CCxxx0_FSCTRL0,  	&(rfSettings.FSCTRL2),1,TYPE_REG);//���Ѽӵ�
    // Write register settings
    write_cc1101(CCxxx0_FSCTRL1,  &(rfSettings.FSCTRL1),1,TYPE_REG);
    write_cc1101(CCxxx0_FSCTRL0,  &(rfSettings.FSCTRL0),1,TYPE_REG);
    write_cc1101(CCxxx0_FREQ2,    	&(rfSettings.FREQ2),1,TYPE_REG);
    write_cc1101(CCxxx0_FREQ1,    	&(rfSettings.FREQ1),1,TYPE_REG);
    write_cc1101(CCxxx0_FREQ0,    	&(rfSettings.FREQ0),1,TYPE_REG);
    write_cc1101(CCxxx0_MDMCFG4,  &(rfSettings.MDMCFG4),1,TYPE_REG);
    write_cc1101(CCxxx0_MDMCFG3,  &(rfSettings.MDMCFG3),1,TYPE_REG);
    write_cc1101(CCxxx0_MDMCFG2,  &(rfSettings.MDMCFG2),1,TYPE_REG);
    write_cc1101(CCxxx0_MDMCFG1,  &(rfSettings.MDMCFG1),1,TYPE_REG);
    write_cc1101(CCxxx0_MDMCFG0,  &(rfSettings.MDMCFG0),1,TYPE_REG);
    write_cc1101(CCxxx0_CHANNR,   	&(rfSettings.CHANNR),1,TYPE_REG);
    write_cc1101(CCxxx0_DEVIATN,  &(rfSettings.DEVIATN),1,TYPE_REG);
    write_cc1101(CCxxx0_FREND1,   &(rfSettings.FREND1),1,TYPE_REG);
    write_cc1101(CCxxx0_FREND0,   &(rfSettings.FREND0),1,TYPE_REG);
    write_cc1101(CCxxx0_MCSM0 ,   &(rfSettings.MCSM0 ),1,TYPE_REG);
    write_cc1101(CCxxx0_FOCCFG,   &(rfSettings.FOCCFG),1,TYPE_REG);
    write_cc1101(CCxxx0_BSCFG,    &(rfSettings.BSCFG),1,TYPE_REG);
    write_cc1101(CCxxx0_AGCCTRL2, &(rfSettings.AGCCTRL2),1,TYPE_REG);
    write_cc1101(CCxxx0_AGCCTRL1, &(rfSettings.AGCCTRL1),1,TYPE_REG);
    write_cc1101(CCxxx0_AGCCTRL0, &(rfSettings.AGCCTRL0),1,TYPE_REG);
    write_cc1101(CCxxx0_FSCAL3,   &(rfSettings.FSCAL3),1,TYPE_REG);
    write_cc1101(CCxxx0_FSCAL2,   &(rfSettings.FSCAL2),1,TYPE_REG);
    write_cc1101(CCxxx0_FSCAL1,   &(rfSettings.FSCAL1),1,TYPE_REG);
    write_cc1101(CCxxx0_FSCAL0,   &(rfSettings.FSCAL0),1,TYPE_REG);
    write_cc1101(CCxxx0_FSTEST,   &(rfSettings.FSTEST),1,TYPE_REG);
    write_cc1101(CCxxx0_TEST2,    &(rfSettings.TEST2),1,TYPE_REG);
    write_cc1101(CCxxx0_TEST1,    &(rfSettings.TEST1),1,TYPE_REG);
    write_cc1101(CCxxx0_TEST0,    &(rfSettings.TEST0),1,TYPE_REG);
    write_cc1101(CCxxx0_IOCFG2,   &(rfSettings.IOCFG2),1,TYPE_REG);
    write_cc1101(CCxxx0_IOCFG0,   &(rfSettings.IOCFG0),1,TYPE_REG);    
    write_cc1101(CCxxx0_PKTCTRL1, &(rfSettings.PKTCTRL1),1,TYPE_REG);
    write_cc1101(CCxxx0_PKTCTRL0, &(rfSettings.PKTCTRL0),1,TYPE_REG);
    write_cc1101(CCxxx0_ADDR,     &(rfSettings.ADDR),1,TYPE_REG);
    write_cc1101(CCxxx0_PKTLEN,   &(rfSettings.PKTLEN),1,TYPE_REG);
	write_cc1101(CCxxx0_SYNC1,   &(rfSettings.SYNC1),1,TYPE_REG);
	write_cc1101(CCxxx0_SYNC0,   &(rfSettings.SYNC0),1,TYPE_REG);
/*
    DEBUG("CCxxx0_FSCTRL0 = %x\r\n",read_cc1101(CCxxx0_FSCTRL0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSCTRL1 = %x\r\n",read_cc1101(CCxxx0_FSCTRL1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FREQ2 = %x\r\n",read_cc1101(CCxxx0_FREQ2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FREQ1 = %x\r\n",read_cc1101(CCxxx0_FREQ1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FREQ0 = %x\r\n",read_cc1101(CCxxx0_FREQ0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MDMCFG4 = %x\r\n",read_cc1101(CCxxx0_MDMCFG4, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MDMCFG3 = %x\r\n",read_cc1101(CCxxx0_MDMCFG3, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MDMCFG2 = %x\r\n",read_cc1101(CCxxx0_MDMCFG2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MDMCFG1 = %x\r\n",read_cc1101(CCxxx0_MDMCFG1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MDMCFG0 = %x\r\n",read_cc1101(CCxxx0_MDMCFG0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_CHANNR = %x\r\n",read_cc1101(CCxxx0_CHANNR, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_DEVIATN = %x\r\n",read_cc1101(CCxxx0_DEVIATN, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FREND1 = %x\r\n",read_cc1101(CCxxx0_FREND1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FREND0 = %x\r\n",read_cc1101(CCxxx0_FREND0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_MCSM0 = %x\r\n",read_cc1101(CCxxx0_MCSM0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FOCCFG = %x\r\n",read_cc1101(CCxxx0_FOCCFG, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_BSCFG = %x\r\n",read_cc1101(CCxxx0_BSCFG, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_AGCCTRL2 = %x\r\n",read_cc1101(CCxxx0_AGCCTRL2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_AGCCTRL1 = %x\r\n",read_cc1101(CCxxx0_AGCCTRL1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_AGCCTRL0 = %x\r\n",read_cc1101(CCxxx0_AGCCTRL0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSCAL3 = %x\r\n",read_cc1101(CCxxx0_FSCAL3, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSCAL2 = %x\r\n",read_cc1101(CCxxx0_FSCAL2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSCAL1 = %x\r\n",read_cc1101(CCxxx0_FSCAL1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSCAL0 = %x\r\n",read_cc1101(CCxxx0_FSCAL0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_FSTEST = %x\r\n",read_cc1101(CCxxx0_FSTEST, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_TEST2 = %x\r\n",read_cc1101(CCxxx0_TEST2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_TEST1 = %x\r\n",read_cc1101(CCxxx0_TEST1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_TEST0 = %x\r\n",read_cc1101(CCxxx0_TEST0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_IOCFG2 = %x\r\n",read_cc1101(CCxxx0_IOCFG2, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_IOCFG0 = %x\r\n",read_cc1101(CCxxx0_IOCFG0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_PKTCTRL1 = %x\r\n",read_cc1101(CCxxx0_PKTCTRL1, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_PKTCTRL0 = %x\r\n",read_cc1101(CCxxx0_PKTCTRL0, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_ADDR = %x\r\n",read_cc1101(CCxxx0_ADDR, RT_NULL, 0,TYPE_STROBE_STATUS));
    DEBUG("CCxxx0_PKTLEN = %x\r\n",read_cc1101(CCxxx0_PKTLEN, RT_NULL, 0,TYPE_STROBE_STATUS));
*/

}
#if 1
void cc1101_send_packet(uint8_t *txBuffer, uint8_t size) 
{
	int i;
	/*DEBUG("<<cc1101 write %d\r\n",size);
    	for(i=0;i<size;i++)
        		DEBUG("%c",txBuffer[i]);*/
	write_cc1101(CCxxx0_TXFIFO, &size,1,TYPE_REG);
    write_cc1101(CCxxx0_TXFIFO, txBuffer, size,TYPE_BURST);
	
	write_cc1101(CCxxx0_STX,RT_NULL,0,TYPE_STROBE_STATUS);
	//while((read_cc1101(CCxxx0_TXBYTES,RT_NULL,0,TYPE_REG)&0x7f)!=0);
	wait_int(RT_TRUE);
    wait_int(RT_FALSE);
	write_cc1101(CCxxx0_SRX,RT_NULL,0,TYPE_STROBE_STATUS);  
	if((read_cc1101(CCxxx0_TXBYTES,RT_NULL,0,TYPE_REG)&0x7f)==0)
	{
		//rt_kprintf(" cc1101 send ok>>\r\n");
		return ;
	}
	rt_kprintf(" cc1101 send failed 2>>\r\n");
}
uint8_t cc1101_rcv_packet(uint8_t *rxBuffer, uint8_t *length) 
{
	#if 0
	uint8_t marc=read_cc1101(CCxxx0_MARCSTATE,RT_NULL,0,TYPE_REG)&0x1f;
	DEBUG("marc %x\r\n",marc);
	if(marc==0x11)
	{
		write_cc1101(CCxxx0_SIDLE,RT_NULL,0,TYPE_STROBE_STATUS);     // Enter IDLE state
		write_cc1101(CCxxx0_SFRX,RT_NULL,0,TYPE_STROBE_STATUS);       // Flush Tx FIFO		
	}
	else{
		marc=read_cc1101(CCxxx0_RXBYTES,RT_NULL,0,TYPE_REG)&0x7f;
		DEBUG("rxbytes %x\r\n",marc);
		if(marc!=0)
		{
			DEBUG("read %d bytes\r\n",read_cc1101(CCxxx0_RXBYTES,RT_NULL,0,TYPE_REG)&0x7f);
			*length=read_cc1101(CCxxx0_RXFIFO,RT_NULL,0,TYPE_STROBE_STATUS);
			read_cc1101(CCxxx0_RXFIFO, rxBuffer, *length,TYPE_BURST); 
		}
		}
	return *length;
	#endif
	uint8_t i,status[2];
	write_cc1101(CCxxx0_SRX,RT_NULL,0,TYPE_STROBE_STATUS);  
	wait_int(RT_TRUE);
	wait_int(RT_FALSE);
	uint8_t marc=read_cc1101(CCxxx0_RXBYTES,RT_NULL,0,TYPE_REG)&0x7f;
	//DEBUG("rxbytes %x\r\n",marc);
	if(marc!=0)
	{
		*length=255;
		uint8_t len=read_cc1101(CCxxx0_RXFIFO,RT_NULL,0,TYPE_STROBE_STATUS);
		//rt_kprintf("\nrcv len is %d\r\n",len);
		if(len<=*length)
			{
				read_cc1101(CCxxx0_RXFIFO, rxBuffer, len,TYPE_BURST);
				read_cc1101(CCxxx0_RXFIFO,status,2,TYPE_BURST);
				if(status[1]&CRC_OK)
				{
					rt_kprintf("\n\n<<cc1101 receive %d bytes\r\n",len);
					for(i=0;i<len;i++)
					rt_kprintf("%c",rxBuffer[i]);
					rt_kprintf("\n");
					//rt_hw_led_on(0);
					//rt_thread_delay(25);
					//rt_hw_led_off(0);
				}
				else
				{
					rt_kprintf("\ncc1101 receive crc failed>>\n");	
					cc1101_hw_init();
					}
				
				*length=len;
				return status[1]&CRC_OK;
			}
		else
			{
				*length=len;
				write_cc1101(CCxxx0_SIDLE,RT_NULL,0,TYPE_STROBE_STATUS);     // Enter IDLE state
				write_cc1101(CCxxx0_SFRX,RT_NULL,0,TYPE_STROBE_STATUS); 
			}
	}
	return 0;
}
#else
void cc1101_send_packet(uint8_t *txBuffer, uint8_t size) 
{
    int i;
    DEBUG("cc1101 write \r\n");
    for(i=0;i<size;i++)
        DEBUG("%x ",txBuffer[i]);

    write_cc1101(CCxxx0_TXFIFO, &size,1,TYPE_REG);
    write_cc1101(CCxxx0_TXFIFO, txBuffer, size,TYPE_BURST);	//д��Ҫ���͵�����

    write_cc1101(CCxxx0_STX,RT_NULL,0,TYPE_STROBE_STATUS);		//���뷢��ģʽ��������

    // Wait for GDO0 to be set -> sync transmitted
    wait_int(RT_TRUE);
    // Wait for GDO0 to be cleared -> end of packet 
    wait_int(RT_FALSE);
    write_cc1101(CCxxx0_SFTX,RT_NULL,0,TYPE_STROBE_STATUS);
}

uint8_t cc1101_rcv_packet(uint8_t *rxBuffer, uint8_t *length) 
{
    uint8_t status[2];
    uint8_t packetLength;
    uint8_t i=(*length)*4;  // �������Ҫ����datarate��length������

    write_cc1101(CCxxx0_SRX,RT_NULL,0,TYPE_STROBE_STATUS);		//�������״̬
    wait_int(RT_FALSE);

    if ((read_cc1101(CCxxx0_RXBYTES,RT_NULL,0,TYPE_STROBE_STATUS) & BYTES_IN_RXFIFO)) //����ӵ��ֽ�����Ϊ0
    {

        packetLength = read_cc1101(CCxxx0_RXFIFO,RT_NULL,0,TYPE_STROBE_STATUS);//������һ���ֽڣ����ֽ�Ϊ��֡���ݳ���

        if (packetLength <= *length) 		//�����Ҫ����Ч���ݳ���С�ڵ��ڽ��յ������ݰ��ĳ���
        {

            read_cc1101(CCxxx0_RXFIFO, rxBuffer, packetLength,TYPE_BURST); //�������н��յ�������
            *length = packetLength;				//�ѽ������ݳ��ȵ��޸�Ϊ��ǰ���ݵĳ���

            // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
            read_cc1101(CCxxx0_RXFIFO, status, 2,TYPE_BURST); 	//����CRCУ��λ

            write_cc1101(CCxxx0_SFRX,RT_NULL,0,TYPE_STROBE_STATUS);		//��ϴ���ջ�����
            DEBUG("cc1101 read \r\n");
            for(i=0;i<*length;i++)
                DEBUG("%x ",rxBuffer[i]);
            write_cc1101(CCxxx0_STX,RT_NULL,0,TYPE_STROBE_STATUS);
            return (status[1] & CRC_OK);			//���У��ɹ����ؽ��ճɹ�
        }
        else 
        {
            *length = packetLength;
            write_cc1101(CCxxx0_SFRX,RT_NULL,0,TYPE_STROBE_STATUS);		//��ϴ���ջ�����
            DEBUG("rx buffer is not enough ,need %d bytes\r\n",packetLength);
            write_cc1101(CCxxx0_STX,RT_NULL,0,TYPE_STROBE_STATUS);
            return 0;
        }
    } 
    else
    {
        write_cc1101(CCxxx0_STX,RT_NULL,0,TYPE_STROBE_STATUS);
        return 0;
    }
}
#endif
void cc1101_hw_init()
{
    reset_cc1101();
    init_rf();
    write_cc1101(CCxxx0_PATABLE,PaTabel,8,TYPE_BURST);
}
