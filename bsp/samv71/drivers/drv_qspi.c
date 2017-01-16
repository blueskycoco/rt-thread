#include <rtthread.h>
#include "board.h"
#include "qspi.h"
#include "drv_qspi.h"
#define PIN_CS  {PIO_PA11, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_IO0  {PIO_PA13A_QIO0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO1  {PIO_PA12A_QIO1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO2  {PIO_PA17A_QIO2, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO3  {PIO_PD31A_QIO3, PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SCK  {PIO_PA14A_QSCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration);
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message);

static struct rt_spi_ops same70_qspi_ops =
{
    configure,
    xfer
};
static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    //struct same70_qspi_bus * same70_qspi_bus = (struct same70_qspi_bus *)device->bus;
	rt_kprintf("configure mode %x, data_width %x, max_hz %d\n",configuration->mode,
		configuration->data_width, configuration->max_hz);
#if 0
    SPI_InitTypeDef SPI_InitStructure;

    SPI_StructInit(&SPI_InitStructure);

    /* data_width */
    if(configuration->data_width <= 8)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    }
    else if(configuration->data_width <= 16)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        return RT_EIO;
    }
    /* baudrate */
    SPI_InitStructure.SPI_BaudRatePrescaler = get_spi_BaudRatePrescaler(configuration->max_hz);
    /* CPOL */
    if(configuration->mode & RT_SPI_CPOL)
    {
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    }
    /* CPHA */
    if(configuration->mode & RT_SPI_CPHA)
    {
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    }
    else
    {
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    }
    /* MSB or LSB */
    if(configuration->mode & RT_SPI_MSB)
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    }
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;

    /* init SPI */
    SPI_I2S_DeInit(same70_qspi_bus->SPI);
    SPI_Init(same70_qspi_bus->SPI, &SPI_InitStructure);
    /* Enable SPI_MASTER */
    SPI_Cmd(same70_qspi_bus->SPI, ENABLE);
    SPI_CalculateCRC(same70_qspi_bus->SPI, DISABLE);
#endif
    return RT_EOK;
};

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    //struct same70_qspi_bus * same70_qspi_bus = (struct same70_qspi_bus *)device->bus;
    struct rt_spi_configuration * config = &device->config;
	int i;
	rt_uint32_t size = message->length;
	rt_kprintf("xfer cs take %d, cs release %d, length %d\n",message->cs_take,
		message->cs_release, message->length);
//	if (message->recv_buf)
//	{
//		rt_uint8_t * recv_ptr = message->recv_buf;
//		for (i=0;i<message->length;i++)
//			*recv_ptr++ = i;
//	}
	Pin *same70_qspi_cs = (Pin *)(&device->parent.user_data);
	
	/* take CS */
	if(message->cs_take)
	    PIO_Clear(same70_qspi_cs);
    if(config->data_width <= 8)
    {
        const rt_uint8_t * send_ptr = message->send_buf;
        rt_uint8_t * recv_ptr = message->recv_buf;

        while(size--)
        {
            rt_uint8_t data = 0x14;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            //while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
            // Send the byte
            //SPI_I2S_SendData(SPI, data);
			//QSPI_WriteSPI(QSPI, data);
			while (!QSPI_GetStatus(QSPI, IsTxEmpty));
			QSPI->QSPI_TDR = data;
			while (!QSPI_GetStatus(QSPI, IsTxSent));            //Wait until a data is received
            //while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
            // Get the received data
            //data = SPI_I2S_ReceiveData(SPI);
			//data = (rt_uint8_t) QSPI_ReadSPI(QSPI);			
			while (!QSPI_GetStatus(QSPI, IsReceived));
			data = (rt_uint8_t)(QSPI->QSPI_RDR);
            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }
    else if(config->data_width <= 16)
    {
        const rt_uint16_t * send_ptr = message->send_buf;
        rt_uint16_t * recv_ptr = message->recv_buf;

        while(size--)
        {
            rt_uint16_t data = 0xFF;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            //while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
            // Send the byte
           // SPI_I2S_SendData(SPI, data);
			//QSPI_WriteSPI(QSPI, data);
			while (!QSPI_GetStatus(QSPI, IsTxEmpty));
			QSPI->QSPI_TDR = data;
			while (!QSPI_GetStatus(QSPI, IsTxSent));            //Wait until a data is received

            //Wait until a data is received
            //while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
            // Get the received data
            //data = SPI_I2S_ReceiveData(SPI);
			//data = QSPI_ReadSPI(QSPI);
			while (!QSPI_GetStatus(QSPI, IsReceived));
			data = (rt_uint16_t)(QSPI->QSPI_RDR);
            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }

	/* release CS */
	if(message->cs_release)
	    PIO_Set(same70_qspi_cs);
	if(message->send_buf){
	rt_uint8_t * send_ptr = &(message->send_buf[0]);
	for (i=0;i<message->length;i++)		
			rt_kprintf("QSPI Snd %x\n", send_ptr[i]);}

	if(message->recv_buf){
	rt_uint8_t * recv_ptr = &(message->recv_buf[0]);
	for (i=0;i<message->length;i++)		
			rt_kprintf("QSPI Rcv %x\n", recv_ptr[i]);}
#if 0
    SPI_TypeDef * SPI = same70_qspi_bus->SPI;
    struct same70_qspi_cs * same70_qspi_cs = device->parent.user_data;
    rt_uint32_t size = message->length;

    /* take CS */
    if(message->cs_take)
    {
        GPIO_ResetBits(same70_qspi_cs->GPIOx, same70_qspi_cs->GPIO_Pin);
    }
    
    if(config->data_width <= 8)
    {
        const rt_uint8_t * send_ptr = message->send_buf;
        rt_uint8_t * recv_ptr = message->recv_buf;

        while(size--)
        {
            rt_uint8_t data = 0xFF;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
            // Send the byte
            SPI_I2S_SendData(SPI, data);

            //Wait until a data is received
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
            // Get the received data
            data = SPI_I2S_ReceiveData(SPI);

            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }
    else if(config->data_width <= 16)
    {
        const rt_uint16_t * send_ptr = message->send_buf;
        rt_uint16_t * recv_ptr = message->recv_buf;

        while(size--)
        {
            rt_uint16_t data = 0xFF;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
            // Send the byte
            SPI_I2S_SendData(SPI, data);

            //Wait until a data is received
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
            // Get the received data
            data = SPI_I2S_ReceiveData(SPI);

            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }
    

    /* release CS */
    if(message->cs_release)
    {
        GPIO_SetBits(same70_qspi_cs->GPIOx, same70_qspi_cs->GPIO_Pin);
    }
#endif
    return message->length;
};

void rt_hw_spi_init(void)
{
	static struct same70_qspi_bus same70_qspi;
	static struct rt_spi_device spi_device;
	static const Pin pinsCS = PIN_CS;	
	static const Pin pQspipins[] = {PIN_IO0, PIN_IO1, PIN_IO2, PIN_IO3, PIN_SCK};
	PIO_Configure(&pinsCS, 1);
	PIO_Configure(pQspipins, PIO_LISTSIZE(pQspipins));
	PMC_EnablePeripheral(ID_QSPI);
	
	QSPI_Disable(QSPI);
	QSPI_SwReset(QSPI);
	//QSPI_ConfigureInterface(QSPI, QSPI_MR_SMM_SPI ,QSPI_MR_NBBITS(QSPI_MR_NBBITS_8_BIT));
	//QSPI_ConfigureMode(QSPI, QSPI_MR_SMM_SPI);
	QSPI->QSPI_MR =  QSPI_MR_SMM_SPI/*|QSPI_MR_LLB_ENABLED*/;
	//QSPI_Configure(QSPI, QSPI_MR_NBBITS(QSPI_MR_NBBITS_8_BIT));
	QSPI->QSPI_MR |=  QSPI_MR_NBBITS(QSPI_MR_NBBITS_8_BIT);
	QSPI_ConfigureClock(QSPI, ClockMode_00, QSPI_SCR_SCBR(1));
	QSPI_Enable(QSPI);
    rt_spi_bus_register(&(same70_qspi.parent), "qspi", &same70_qspi_ops);
	rt_spi_bus_attach_device(&spi_device, "spi10", "qspi", (void*)&pinsCS);
}
