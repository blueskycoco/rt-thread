/*
 *	brife: this is at88sc common code 
 */
#include "s1.h"
/*define RA to RG as 5 bit width,SA to SG as 7 bit width,TA to TE as 5 bit width register*/
unsigned char RA=0;unsigned char RB=0;unsigned char RC=0;unsigned char RD=0;unsigned char RE=0;unsigned char RF=0;unsigned char RG=0;
unsigned char SA=0;unsigned char SB=0;unsigned char SC=0;unsigned char SD=0;unsigned char SE=0;unsigned char SF=0;unsigned char SG=0;
unsigned char TA=0;unsigned char TB=0;unsigned char TC=0;unsigned char TD=0;unsigned char TE=0;
/*define Gpa_byte used by all functions*/
unsigned char Gpa_byte=0;
BOOL needDecry=FALSE;
BOOL needAuth=FALSE;
void i2c_SendStart(void)
{
	i2c_sda_set(1);
	i2c_scl_set(1);
	i2c_sda_set(0);
	i2c_scl_set(0);
}
void i2c_SendStop(void)
{
	i2c_sda_set(0);
	i2c_scl_set(1);
	i2c_sda_set(1);
}
void i2c_SendAck(void)
{
	i2c_sda_set(0);
	i2c_scl_set(0);
	i2c_scl_set(1);
	i2c_scl_set(0);
}
void i2c_SendNack(void)
{
	i2c_sda_set(1);
	i2c_scl_set(0);
	i2c_scl_set(1);
	i2c_scl_set(0);
}
void i2c_SendData(unsigned char data)
{
	unsigned char bit;

	for (bit = 0x80; bit != 0; bit >>= 1)
	{   
		if (data & bit)
		{		    
			i2c_sda_set(1);
		} 
		else
		{	
			i2c_sda_set(0);
		}
		i2c_scl_set(1);
		i2c_scl_set(0);		
	}
}
unsigned char i2c_ReceiveAck(void)
{
	unsigned char bit;

	/* Set I2C_DAT line as output */
	i2c_sda_input();

	i2c_scl_set(1);
	bit = i2c_sda_get();
	i2c_scl_set(0);

	/* Set I2C_DAT line as input */
	i2c_sda_output();

	return !bit;
}
unsigned char i2c_ReceiveData(void)
{
	unsigned char data = 0;
	unsigned char x;

	/* make sure the data line is released */
	i2c_sda_input();
	/* shift in the data 
	*/
	for (x = 0; x < 8; x++)
	{   
		/* shift the data left */
		i2c_scl_set(1);
		data <<= 1;
		data |= i2c_sda_get();
		i2c_scl_set(0);
	}
	i2c_sda_output();
	return data;                        
}
/*cm_PowerOn the first lowlevel function will be called*/
void cm_PowerOn(void)
{
	static BOOL flag=FALSE;
	int i=0;
	if(flag==FALSE)
	{
		/* init SIO low-level abstraction layer */
		i2c_init();
		/* set the clock and data lines to the proper states */
		i2c_scl_set(1);
		i2c_sda_set(1);
		sleep_ms(10);
		for(i=0;i<6;i++)
		{
			i2c_scl_set(1);
			sleep_ms(1);
			i2c_scl_set(0);
			sleep_ms(1);
		}
		i2c_scl_set(1);
		flag=TRUE;
	}
}
BOOL cm_Read(unsigned char Command, unsigned char Addr1, unsigned char Addr2,unsigned char Nbytes,unsigned char *pBuffer)
{
	unsigned char i;
	int restart_count = 0;

	unsigned char *pReadPtr = pBuffer;

	if (!pBuffer)
	{	    
		return FALSE;
	}		

	if (Nbytes > BYTES_MAX)
	{
		AT88DBG("<cm_Read>Warning: read too many bytes (%d) a time!\r\n", Nbytes);
		return FALSE;
	}

	//AT88DBG("<cm_Read>Command = 0x%02x, Addr1 = 0x%02x, Addr2 = 0x%02x, Nbytes = 0x%02x\r\n", Command, Addr1,Addr2,Nbytes);
	//sleep_ms(200UL);

	while (restart_count++ < 5)
	{
		/* If restart, revert to the start of buffer! */
		pReadPtr = pBuffer;
		/*
		 * send out command,addr1,addr2,Nbytes.
		 */		
		i2c_SendStart();
		i2c_SendData(Command);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Read>NACK received after Command.\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		i2c_SendData(Addr1);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Read>NACK received after Addr1.\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		i2c_SendData(Addr2);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Read>NACK received after Addr2.\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}	
		i2c_SendData(Nbytes);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Read>NACK received after Nbytes.\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		for (i = 0; i < Nbytes; i++)
		{		   	
			*pReadPtr++ = i2c_ReceiveData();       
			if(i<(Nbytes-1))
				i2c_SendAck();        	        
		}

		/* After read bytes done, send a STOP signal to the I2C bus
		*/
		i2c_SendNack();
		i2c_SendStop();
		//sleep_ms(200UL);
		//AT88DBG("[SIO_READ]receive data.\n");
		//for(i=0;i<Nbytes;i++)
		//AT88DBG("%x ",pBuffer[i]);
		return TRUE;
	}

	AT88DBG("<cm_Read>(0x%x) failed, too many NACKs.\r\n", Command);	
	return FALSE;
}

unsigned int cm_Write(unsigned char Command, unsigned char Addr1, unsigned char Addr2,unsigned char Nbytes,unsigned char *pBuffer)
{
	unsigned long i;
	int restart_count = 0;
	unsigned char *pWritePtr = pBuffer;
	//unsigned char rdData[BYTES_MAX];
	//AT88DBG("<cm_Write> Command = 0x%02x, Addr1 = 0x%02x, Addr2 = 0x%02x, Nbytes = 0x%02x,pBuffer %x %x\r\n", 
	// Command, Addr1, Addr2,Nbytes,pBuffer[0],pBuffer[1]);


	if (Nbytes > BYTES_MAX)
	{
		AT88DBG("<cm_Write>Error: write too many bytes (%d) a time!\r\n", Nbytes);
		return 0;
	}

	while (restart_count++ < 5)
	{
		/* If restart, revert to the start of buffer! */
		if(pBuffer !=NULL && Nbytes != 0)
			pWritePtr = pBuffer;

		/*
		 * send out command,addr1,addr2,Nbytes
		 */
		i2c_SendStart();
		/* Write device address */
		i2c_SendData(Command);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Write>NACK received after Command. %x\r\n",Command);
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		i2c_SendData(Addr1);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Write>NACK received after Addr1.\r\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		i2c_SendData(Addr2);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Write>NACK received after Addr2.\r\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		i2c_SendData(Nbytes);
		if (!i2c_ReceiveAck())
		{
			AT88DBG("<cm_Write>NACK received after Nbytes.\r\n");
			i2c_SendStop();
			sleep_ms(20UL);
			continue;
		}
		/* Following bytes are written in successive internal registers */
		if(pBuffer !=NULL && Nbytes != 0)
		{
			for (i = 0; i < Nbytes; i++)
			{
				i2c_SendData(*pWritePtr++);
				if (!i2c_ReceiveAck())
				{
					AT88DBG("<cm_Write>NACK received after %x data byte.\r\n", i+1);
					i2c_SendStop();
					sleep_ms(20UL);
					break;
				}            
			}
			if(i<Nbytes)
			{
				continue;
			}
		}

		/* done. */
		i2c_SendStop();

		return Nbytes;

	}

	AT88DBG("<cm_Write>(0x%x) failed, too many NACKs.\r\n", Addr2);
	return 0;
}
void cm_AckPolling(unsigned char Command)
{
	//Acknowledge Polling
	i2c_SendStart();
	/* Write device address */
	i2c_SendData(Command);
	while (!i2c_ReceiveAck())
	{
		//AT88DBG("<cm_AckPolling>Acknowledge Polling\r\n");
		i2c_SendStart();
		i2c_SendData(Command);
		sleep_ms(2UL);        
	}
	i2c_SendStop();
}

void clock_gpa(unsigned char Datain)
{
	unsigned char Din_gpa,Ri,Si,Ti;
	/*define reg RGin,Raddmod31,Taddmod31 as 5 bit width register*/
	unsigned char RGin,Raddmod31,Taddmod31;
	/*define reg SGin,Saddmod127 as 7 bit width register*/
	unsigned char SGin,Saddmod127;
	/*define reg Gpaoi as 4 bit width register*/
	unsigned char Gpaoi=0;
	Din_gpa = Datain ^ Gpa_byte;
	Ri = Din_gpa & 0x1f;
	Si = (((Din_gpa & 0x0f)<<3 | (Din_gpa & 0xe0)>>5)) & 0x7f;
	Ti = ((Din_gpa & 0xf8) >> 3 ) & 0x1f ;
	//R Polynomial 
	RGin = ((RG & 0xf) <<1 | (RG & 0x10) >>4)&0x1f;

	if(RD + RGin > 31)
		Raddmod31 = RD+RGin-31;
	else
		Raddmod31 = RD+RGin;

	RG = RF;
	RF = RE;
	RE = RD;
	RD = RC ^ Ri;
	RC = RB;
	RB = RA;
	RA = Raddmod31;
	//S Polynomial
	SGin = ((SG & 0x3f) <<1 | (SG & 0x40) >>6)&0x7f;

	if(SF + SGin > 127)
		Saddmod127 = SF+SGin-127;
	else
		Saddmod127 = SF+SGin;

	SG = SF;
	SF = SE^Si;
	SE = SD;
	SD = SC;
	SC = SB;
	SB = SA;
	SA = Saddmod127;

	//T Polynomial
	if(TE + TC > 31)
		Taddmod31 = TE+TC-31;
	else
		Taddmod31 = TE+TC;

	TE = TD;
	TD = TC;
	TC = TB^Ti;
	TB = TA;
	TA = Taddmod31;
	//Output stage
	Gpaoi = ((~SA & (RA ^ RE)) | (SA & (TA ^TD))) & 0x0f;
	Gpa_byte = ((Gpa_byte&0x0f)<<4) | Gpaoi;
}
void clock_gpaXtimes(unsigned char Datain,unsigned char times)
{
	int i=0;
	for(i=0;i<times;i++)
		clock_gpa(Datain);

}
BOOL cm_SendChecksum(unsigned char ucDevAddr)
{
	unsigned char pucChkSum[2];
	if(needAuth)
	{
		clock_gpaXtimes(0x00,15);
		pucChkSum[0] = Gpa_byte;
		clock_gpaXtimes(0x00,5);
		pucChkSum[1] = Gpa_byte;
	}
	//AT88DBG("<cm_SendChecksum>%x %x\r\n",pucChkSum[0],pucChkSum[1]);
	if(cm_Write((ucDevAddr<<4)|0x04, 0x02, 0x00,0x02,pucChkSum)!=0x02)
		return FALSE;
	else
	{
		cm_AckPolling((ucDevAddr<<4)|0x02);
		return TRUE;
	}
}
void cm_resetCryptoVal(void)
{
	RA=0;
	RB=0;
	RC=0;
	RD=0;
	RE=0;
	RF=0;
	RG=0;
	SA=0;
	SB=0;
	SC=0;
	SD=0;
	SE=0;
	SF=0;
	SG=0;
	TA=0;
	TB=0;
	TC=0;
	TD=0;
	TE=0;
	Gpa_byte=0;
	needDecry=FALSE;
	needAuth=FALSE;
}

BOOL cm_WriteConfigZone(unsigned char ucDevAddr, unsigned char ucCryptoAddr, unsigned char* pucBuffer, unsigned char ucCount,unsigned char ucAntiTearing)
{

	int i;
	unsigned char tmp;
	if(needAuth)
	{
		clock_gpaXtimes(0x00, 5);
		clock_gpaXtimes(ucCryptoAddr,1);
		clock_gpaXtimes(0x00,5);
		clock_gpaXtimes(ucCount,1);

		for(i=0;i<ucCount;i++)
		{
			clock_gpaXtimes(0x00,5);
			tmp=pucBuffer[i];
			//we need encrypt password zone
			if((ucCryptoAddr+i) <= (AT88SC_R7+2) && (ucCryptoAddr+i)>=AT88SC_PACW0)
				pucBuffer[i] = pucBuffer[i] ^ Gpa_byte;

			clock_gpaXtimes(tmp,1);	
		}
	}

	if(cm_Write((ucDevAddr<<4)|0x04, 0x00, ucCryptoAddr,ucCount,pucBuffer)!=ucCount)
		return FALSE;
	cm_AckPolling((ucDevAddr<<4)|0x02);
	return TRUE;

}
BOOL cm_ReadConfigZone(unsigned char ucDevAddr, unsigned char ucCryptoAddr, 
		unsigned char* pucBuffer, unsigned char ucCount)
{
	int i;

	if(needAuth)
	{
		clock_gpaXtimes(0x00, 5);
		clock_gpaXtimes(ucCryptoAddr,1);
		clock_gpaXtimes(0x00,5);
		clock_gpaXtimes(ucCount,1);
	}
	if(!cm_Read((ucDevAddr<<4)|0x06, 0x00, ucCryptoAddr,ucCount,pucBuffer))
		return FALSE;
	//we need decry password zone , if we read them
	if(needAuth)
		for(i=0;i<ucCount;i++)
		{
			if((ucCryptoAddr+i) <= (AT88SC_R7+2) && (ucCryptoAddr+i)>=AT88SC_PACW0)
				pucBuffer[i] = pucBuffer[i] ^ Gpa_byte;
			clock_gpaXtimes(pucBuffer[i],1);
			clock_gpaXtimes(0x00,5);
		}
	return TRUE;
}
BOOL cm_VerifyPassword(unsigned char ucDevAddr, unsigned char* pucPassword, unsigned char ucSet, 
		unsigned char ucRW)
{
	int i;
	unsigned char epucPassword[3];
	unsigned char verifyok;
	unsigned char ucSetAddr = AT88SC_PACW0;
	if(needAuth)
	{
		for(i=0;i<3;i++)
		{
			clock_gpaXtimes(pucPassword[i], 5);
			epucPassword[i] = Gpa_byte;
		}
	}
	else
	{
		for(i=0;i<3;i++)
		{
			epucPassword[i] = pucPassword[i];
		}

	}
	switch(ucSet)
	{
		case 0:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW0;
			else
				ucSetAddr = AT88SC_PACR0;
			break;
		case 1:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW1;
			else
				ucSetAddr = AT88SC_PACR1;
			break;
		case 2:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW2;
			else
				ucSetAddr = AT88SC_PACR2;
			break;
		case 3:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW3;
			else
				ucSetAddr = AT88SC_PACR3;
			break;
		case 4:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW4;
			else
				ucSetAddr = AT88SC_PACR4;
			break;
		case 5:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW5;
			else
				ucSetAddr = AT88SC_PACR5;
			break;
		case 6:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW6;
			else
				ucSetAddr = AT88SC_PACR6;
			break;
		case 7:
			if(!ucRW)
				ucSetAddr = AT88SC_PACW7;
			else
				ucSetAddr = AT88SC_PACR7;
			break;
		default:
			break;
	}
	if(cm_Write((ucDevAddr<<4)|0x0a, (ucRW<<4)|ucSet, 0x00,0x03,epucPassword)!=0x03)
		return FALSE;
	else
	{
		//if(needAuth)
		cm_AckPolling((ucDevAddr<<4)|0x02);
		cm_ReadConfigZone(ucDevAddr, ucSetAddr, &verifyok, 1);
		if(verifyok!=0xff)
		{
			AT88DBG("Verify %x Password failed %x\n",ucSet,verifyok);
			return FALSE;
		}


	}
	return TRUE;
}
BOOL cm_ReadChecksum(unsigned char ucDevAddr)
{
	unsigned char dcr;
	unsigned char pucChkSum[2];

	if(needAuth)
	{
		clock_gpaXtimes(0x00,10);
		pucChkSum[0] = Gpa_byte;
		clock_gpaXtimes(0x00,5);
		pucChkSum[1] = Gpa_byte;
		clock_gpaXtimes(0x00,5);
		AT88DBG("Computed CheckSum is %x %x \n",pucChkSum[0],pucChkSum[1]);
	}

	if(!cm_Read((ucDevAddr<<4)|0x06, 0x02, 0x00,0x02,pucChkSum))
		return FALSE;
	else
	{

		AT88DBG("<cm_ReadChecksum>%x %x\r\n",pucChkSum[0],pucChkSum[1]);
		cm_ReadConfigZone(ucDevAddr, AT88SC_DCR, &dcr, 1);
		if(dcr & 0x40)
		{
			AT88DBG("ReadCheckSum failed\n");
			return FALSE;
		}
	}
	return TRUE;
}
BOOL cm_SetUserZone(unsigned char ucDevAddr, unsigned char ucZoneNumber, 
		unsigned char ucAntiTearing)
{
	if(needAuth)
		clock_gpaXtimes(ucZoneNumber,1);
	cm_Write((ucDevAddr<<4)|0x04, 0x03, ucZoneNumber,0x00,NULL);
	return TRUE;

}
BOOL cm_WriteUserZone(unsigned char ucDevAddr, unsigned int uiCryptoAddr, 
		unsigned char* pucBuffer, unsigned char ucCount)
{

	int i;
	unsigned char tmp;
	if(needAuth)
	{
		clock_gpaXtimes(0x00, 5);
		clock_gpaXtimes(uiCryptoAddr,1);
		clock_gpaXtimes(0x00,5);
		clock_gpaXtimes(ucCount,1);

		for(i=0;i<ucCount;i++)
		{
			clock_gpaXtimes(0x00,5);
			tmp=pucBuffer[i];
			if(needDecry)
				pucBuffer[i] = pucBuffer[i] ^ Gpa_byte;					
			clock_gpaXtimes(tmp,1);
		}
	}

	if(cm_Write((ucDevAddr<<4)|0x00, 0x00, uiCryptoAddr,ucCount,pucBuffer)!=ucCount)
		return FALSE;
	if(needAuth)
	{
		if(!cm_SendChecksum(ucDevAddr))
			return FALSE;
	}
	else
		cm_AckPolling((ucDevAddr<<4)|0x02);
	return TRUE;

}
BOOL cm_ReadFuse(unsigned char ucDevAddr, unsigned char* pucFuze)
{
	clock_gpaXtimes(0x00, 5);
	clock_gpaXtimes(0x00, 1);
	clock_gpaXtimes(0x00, 5);
	clock_gpaXtimes(0x01, 1);
	cm_Read((ucDevAddr<<4)|0x06, 0x01, 0x00,0x01,pucFuze);
	clock_gpaXtimes(*pucFuze, 1);
	clock_gpaXtimes(0x00, 5);
	return TRUE;
}
BOOL cm_WriteFuse(unsigned char ucDevAddr, unsigned char* pucFuze)
{
	BOOL result = cm_Write((ucDevAddr<<4)|0x04, 0x01, *pucFuze,0x00,NULL);
	cm_AckPolling((ucDevAddr<<4)|0x02);
	return result;
}


BOOL cm_ReadUserZone(unsigned char ucDevAddr, unsigned int uiCryptoAddr, 
		unsigned char* pucBuffer, unsigned char ucCount)
{
	int i;

	if(needAuth)
	{
		clock_gpaXtimes(0x00, 5);
		clock_gpaXtimes(uiCryptoAddr,1);
		clock_gpaXtimes(0x00,5);
		clock_gpaXtimes(ucCount,1);
	}
	if(!cm_Read((ucDevAddr<<4)|0x02, 0x00, uiCryptoAddr,ucCount,pucBuffer))
		return FALSE;
	//we need decry password zone , if we read them
	if(needAuth)
	{
		for(i=0;i<ucCount;i++)
		{
			if(needDecry)
			{
				pucBuffer[i] = pucBuffer[i] ^ Gpa_byte;
			}
			clock_gpaXtimes(pucBuffer[i],1);
			clock_gpaXtimes(0x00,5);
		}
	}
	return TRUE;

}

BOOL cm_VerifyCrypto(unsigned char ucDevAddr, unsigned char ucKeySet, unsigned char* pucKey)
{
	unsigned char buf[16];
	int i;
	unsigned char Ci[8];
	unsigned char Sk[8];
	unsigned char NewCi[8];
	unsigned char CiAddr=AT88SC_CI0,SkAddr=AT88SC_SK0;

	for(i=0;i<8;i++)
		buf[i]=i*12+3;
	switch(ucKeySet)
	{
		case 0:
			CiAddr=AT88SC_CI0;
			SkAddr=AT88SC_SK0;
			break;
		case 1:
			CiAddr=AT88SC_CI1;
			SkAddr=AT88SC_SK1;
			break;
		case 2:
			CiAddr=AT88SC_CI2;
			SkAddr=AT88SC_SK2;
			break;
		case 3:
			CiAddr=AT88SC_CI3;
			SkAddr=AT88SC_SK3;
			break;
		default:
			break;
	}
	//readback Ci and Sk
	cm_ReadConfigZone(DEFAULT_ADDRESS, CiAddr-1, Ci, 8);    
	cm_resetCryptoVal();
	//Verify init
	for(i=0;i<4;i++)
	{
		clock_gpaXtimes(Ci[2*i], 3);
		clock_gpaXtimes(Ci[2*i+1], 3);
		clock_gpaXtimes(buf[i], 1);
	}
	for(i=0;i<4;i++)
	{
		clock_gpaXtimes(pucKey[2*i], 3);
		clock_gpaXtimes(pucKey[2*i+1], 3);
		clock_gpaXtimes(buf[i+4], 1);
	}
	clock_gpaXtimes(0x00,6);
	buf[8] = Gpa_byte;//challenge 0
	for(i=1;i<8;i++)
	{
		clock_gpaXtimes(0x00,7);
		buf[i+8] = Gpa_byte;//challenge 1 to 7
	}
	//send challenge and random number
	if(cm_Write((ucDevAddr<<4)|0x08, ucKeySet, 0x00,0x10,buf)!=0x10)
	{
		AT88DBG("cm_Write failed\n");
		return FALSE;
	}
	else
	{
		//compute new Ci and new Sk
		Ci[0]=0xff;
		for(i=1;i<8;i++)
		{
			clock_gpaXtimes(0x00, 2);
			Ci[i]=Gpa_byte;
		}
		for(i=0;i<8;i++)
		{
			clock_gpaXtimes(0x00,2);
			Sk[i]=Gpa_byte;
		}
		clock_gpaXtimes(0x00,3);
		needAuth=TRUE;
		cm_AckPolling((ucDevAddr<<4)|0x06);
		cm_ReadConfigZone(DEFAULT_ADDRESS, CiAddr-1, NewCi, 8);
		for(i=0;i<8;i++)
		{
			if(Ci[i]!=NewCi[i])
			{
				AT88DBG("Ci[%d] = %x ,NewCi[%d] = %x\n",i,Ci[i],i,NewCi[i]);
				needDecry=FALSE;
				needAuth=FALSE;
				return FALSE;
			}
		}
	}


	//begin Encryption authication
	cm_resetCryptoVal();
	for(i=0;i<4;i++)
	{
		clock_gpaXtimes(Ci[2*i], 3);
		clock_gpaXtimes(Ci[2*i+1], 3);
		clock_gpaXtimes(buf[i], 1);
	}
	for(i=0;i<4;i++)
	{
		clock_gpaXtimes(Sk[2*i], 3);
		clock_gpaXtimes(Sk[2*i+1], 3);
		clock_gpaXtimes(buf[i+4], 1);
	}
	clock_gpaXtimes(0x00,6);
	buf[8] = Gpa_byte;//challenge 0
	for(i=1;i<8;i++)
	{
		clock_gpaXtimes(0x00,7);
		buf[i+8] = Gpa_byte;//challenge 1 to 7
	}
	//send challenge and random number
	if(cm_Write((ucDevAddr<<4)|0x08, 0x10|ucKeySet, 0x00,0x10,buf)!=0x10)
	{
		AT88DBG("cm_Write failed 2\n");
		return FALSE;
	}
	else
	{
		//compute new Ci and new Sk
		Ci[0]=0xff;
		for(i=1;i<8;i++)
		{
			clock_gpaXtimes(0x00, 2);
			Ci[i]=Gpa_byte;
		}
		for(i=0;i<8;i++)
		{
			clock_gpaXtimes(0x00,2);
			Sk[i]=Gpa_byte;
		}
		clock_gpaXtimes(0x00,3);
		needAuth=TRUE;
		cm_AckPolling((ucDevAddr<<4)|0x06);
		cm_ReadConfigZone(DEFAULT_ADDRESS, CiAddr-1, NewCi, 8);
		for(i=0;i<8;i++)
		{
			if(Ci[i]!=NewCi[i])
			{
				AT88DBG("Ci[%d] = %x ,NewCi[%d] = %x\n",i,Ci[i],i,NewCi[i]);
				return FALSE;
			}
		}         
		needDecry=TRUE;
		needAuth=TRUE;        
	}

	return TRUE;
}
/*
   BOOL auth(pge p,callback_t cb)
   {
   BOOL ucReturn;
   unsigned char i;	
   unsigned char user_zone[32];
   cm_PowerOn();

   ucReturn = cm_VerifyCrypto(DEFAULT_ADDRESS, p->use_g, p->g);
   if (ucReturn != TRUE){
//AT88DBG("cm_VerifyCrypto failed1\n");
return FALSE;
}
ucReturn = cm_VerifyPassword(DEFAULT_ADDRESS, p->pw,p->use_pw, 0);
if (ucReturn != TRUE)  {
//AT88DBG("cm_VerifyPassword failed\n");
return FALSE;
}
cm_SetUserZone(DEFAULT_ADDRESS, p->zone_index, FALSE);
memset(user_zone,0xff,32*sizeof(unsigned char));
ucReturn = cm_ReadUserZone(DEFAULT_ADDRESS, 0, user_zone, 32);
if (ucReturn != TRUE){
//AT88DBG("At88sc_Read failed %d\n",p->zone_index);
return FALSE;
}
for(i=0;i<32;i++)
{
if(p->user_zone[i]!=user_zone[i])
return FALSE;
}
cb();
return TRUE;
}
*/

BOOL get_config(unsigned char *buf)
{
	BOOL ucReturn;	
	unsigned char ucData[240];
	unsigned char Def_SecureCode[3] = {0xdd,0x42,0x97};
	int i;
	unsigned char fuse;	
	cm_PowerOn();
	ucReturn = cm_ReadConfigZone(DEFAULT_ADDRESS, AT88SC_ATR, buf, 24);
	if (ucReturn != TRUE) 
	{
		AT88DBG("cm_ReadConfigZone failed\n");
		return FALSE;
	}
	#if BURN
	cm_ReadFuse(DEFAULT_ADDRESS,&fuse);
	AT88DBG("fuse , %x",fuse);	
	/*unlock config area*/
	ucReturn = cm_VerifyPassword(DEFAULT_ADDRESS, Def_SecureCode,7, 0);
	if (ucReturn != TRUE)  
	{
		AT88DBG("cm_VerifyPassword failed\n");
		return FALSE;
	}

	ucReturn = cm_ReadConfigZone(DEFAULT_ADDRESS, AT88SC_ATR, ucData, 0xf0);
	if (ucReturn != TRUE) 
	{
		AT88DBG("cm_ReadConfigZone failed\n");
		return FALSE;
	}
	AT88DBG("\nRead all config data again:\n");
	for(i=0;i<0xf0;i++)
	{
		if(i%8==0 && i!=0)
			AT88DBG("\n");
		AT88DBG("%4X ",ucData[i]);		
	}
	#endif
	return ucReturn;
}

#if BURN
BOOL burn(pe p)
{
	unsigned char ucData[240];
	unsigned char Def_SecureCode[3] = {0xdd,0x42,0x97};
	unsigned char Def_SecureCode2[3] = {0xff,0xff,0xff};
	unsigned char Def_SecureCode_fuse[3] = {0x32,0x56,0x9a};
	unsigned char g[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	unsigned char pw[3]={0xff,0xff,0xff};
	BOOL ucReturn;
	unsigned char i,addr;	
	unsigned char fuse;	
	cm_PowerOn();    
	//1 test iic bus
	ucData[0] = 0x77;
	ucData[1] = 0x33;
	ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_MTZ, ucData, 2, FALSE);
	if (ucReturn != TRUE) {
		AT88DBG("Write Config Zone MTZ failed\n");
		return FALSE;
	}
	// Read back data
	ucData[0] = 0x00;
	ucData[1] = 0x00;
	ucReturn = cm_ReadConfigZone(DEFAULT_ADDRESS, AT88SC_MTZ, ucData, 2);
	if (ucReturn != TRUE || ucData[0]!=0x77 || ucData[1]!=0x33) {
		AT88DBG("Read Config Zone MTZ failed\n");
		return FALSE;
	}   
	memset(ucData,0xff,240*sizeof(unsigned char));	
	cm_ReadFuse(DEFAULT_ADDRESS,&fuse);
	AT88DBG("fuse , %x",fuse);
	if((fuse&0x7)==0x0)
	{
		AT88DBG("have already fused , can not write config area\r\n");
		return TRUE;//we cant not update config zone after fuse ,except pw and userzone
	}
	//5 unlock config zone , begin to write config zone
	ucReturn = cm_VerifyPassword(DEFAULT_ADDRESS, Def_SecureCode,7, 0);
	if (ucReturn != TRUE)  {
		AT88DBG("cm_VerifyPassword failed\n");
		return FALSE;
	}
	ucReturn = cm_ReadConfigZone(DEFAULT_ADDRESS, AT88SC_ATR, ucData, 0xf0);
	if (ucReturn != TRUE) {
		AT88DBG("cm_ReadConfigZone failed\n");
		return FALSE;
	}
	AT88DBG("\nRead all config data fist time\n");
	for(i=0;i<0xf0;i++)
	{
		if(i%8==0 && i!=0)
			AT88DBG("\n");
		AT88DBG("%4X ",ucData[i]);		
	}

	memset(ucData,0xff,240*sizeof(unsigned char));	
	//6 write pw
	ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_W0, p.pw, 7, FALSE);
	if(ucReturn != TRUE) {
		AT88DBG("cm_WriteConfigZone PW failed \n");
		return FALSE;
	}
	//7 write G
	ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_G0, p.g, 8, FALSE);
	if (ucReturn != TRUE) {
		AT88DBG("cm_WriteConfigZone G Failed\n");
		return FALSE;
	}
	//8 write Ci
	ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_CI0, p.ci, 7, FALSE);
	if (ucReturn != TRUE) {
		AT88DBG("cm_WriteConfigZone Ci Failed\n");
		return FALSE;
	}
    for(i=0;i<p.num_ar;i++)
    {
        
        ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_AR0+i*2, p.ar, 2, FALSE);
        if (ucReturn != TRUE) {
            AT88DBG("cm_WriteConfigZone AR Failed\n");
            return FALSE;
        }
        
    }
	//10 write id
	ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_INN, p.id, 7, FALSE);
	if (ucReturn != TRUE) {
		AT88DBG("cm_WriteConfigZone ID Failed\n");
		return FALSE;
	}

	if(p.fuse==TRUE)
	{//10 fuse chip or not
		//set fuse
		unsigned char fuse;
		//update Def_SecureCode before fuse
		//unsigned char pw[7]={0x32,0x56,0x9a,0xff,0xff,0xff,0xff};
		ucReturn = cm_WriteConfigZone(DEFAULT_ADDRESS, AT88SC_W7, p.New_SecureCode, 7, FALSE);
		if (ucReturn != TRUE) {
			AT88DBG("cm_WriteConfigZone Def_SecureCode Failed\n");
			return FALSE;
		}
		cm_ReadFuse(DEFAULT_ADDRESS,&fuse);
		AT88DBG("Before fuse , %x",fuse);
		fuse=0x06;
		cm_WriteFuse(DEFAULT_ADDRESS,&fuse);
		fuse=0x04;
		cm_WriteFuse(DEFAULT_ADDRESS,&fuse);
		fuse=0x00;
		cm_WriteFuse(DEFAULT_ADDRESS,&fuse);
		cm_ReadFuse(DEFAULT_ADDRESS,&fuse);
		AT88DBG("after fuse , %x",fuse);
	}

	ucReturn = cm_ReadConfigZone(DEFAULT_ADDRESS, AT88SC_ATR, ucData, 0xf0);
	if (ucReturn != TRUE) {
		AT88DBG("cm_ReadConfigZone failed\n");
		return FALSE;
	}
	AT88DBG("\nRead all config data again\n");
	for(i=0;i<0xf0;i++)
	{
		if(i%8==0 && i!=0)
			AT88DBG("\n");
		AT88DBG("%4X ",ucData[i]);		
	}
	return TRUE;
}
#else

BOOL userzone_proc(pge p,BOOL read)
{
	BOOL ucReturn;
	unsigned char ucData[240];

	unsigned char i,index=0;	
	cm_PowerOn();
	ucReturn = cm_VerifyCrypto(DEFAULT_ADDRESS, p->use_g, p->g);
	if (ucReturn != TRUE)
	{
		AT88DBG("cm_VerifyCrypto failed1\n");
		return FALSE;
	}
	ucReturn = cm_VerifyPassword(DEFAULT_ADDRESS, p->pw,p->use_pw, 0);
	if (ucReturn != TRUE)  
	{
		AT88DBG("cm_VerifyPassword failed\n");
		return FALSE;
	}
	
	cm_SetUserZone(DEFAULT_ADDRESS, p->zone_index, FALSE);
	if(read)
	{
		ucReturn = cm_ReadUserZone(DEFAULT_ADDRESS, p->addr, p->user_zone, p->len);
		if (ucReturn != TRUE)
		{
		AT88DBG("cm_ReadUserZone failed %d\n",p->zone_index);
		return FALSE;
		}
	}
	else
	{
		for(i=0;i<p->len/p->page_size;i++)
		{
			ucReturn = cm_WriteUserZone(DEFAULT_ADDRESS, p->addr+index,(unsigned char *)(p->user_zone+index), p->page_size);
			if(ucReturn != TRUE)
			{
				AT88DBG("cm_WriteUserZone Zone failed 1\n");
				return FALSE;
			}
			index=index+p->page_size;
		}
		if((p->len%p->page_size)!=0)
		{
			ucReturn = cm_WriteUserZone(DEFAULT_ADDRESS, p->addr+index, (unsigned char *)(p->user_zone+index), p->len-index);
			if(ucReturn != TRUE)
			{
				AT88DBG("cm_WriteUserZone Zone failed 2\n");
				return FALSE;
			}
		}

	}
return TRUE;
}
#endif
