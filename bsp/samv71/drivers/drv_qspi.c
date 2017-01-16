#include <rtthread.h>
#include "board.h"
#include "qspi.h"
#include "drv_qspi.h"
#define PIN_CS   {PIO_PA11A_QCS,  PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO0  {PIO_PA13A_QIO0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO1  {PIO_PA12A_QIO1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO2  {PIO_PA17A_QIO2, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_IO3  {PIO_PD31A_QIO3, PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SCK  {PIO_PA14A_QSCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration);
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message);

Qspid_t qspi;

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
    struct rt_spi_configuration * config = &device->config;
	int i;
	
	//const rt_uint8_t * send_ptr = message->send_buf;
	//rt_uint8_t * recv_ptr = message->recv_buf;

	if(message->send_buf != RT_NULL)
	{
		qspi.qspiBuffer.pDataTx = 
			message->send_buf;
		qspi.qspiBuffer.TxDataSize = 
			message->length;
		QSPI_MultiWriteSPI(&qspi,(uint16_t const *)qspi.qspiBuffer.pDataTx,
					   qspi.qspiBuffer.TxDataSize); 
	}

	if(message->recv_buf != RT_NULL)
	{
		qspi.qspiBuffer.pDataRx = 
			message->recv_buf;
		qspi.qspiBuffer.RxDataSize = 
			message->length;
		QSPI_MultiReadSPI(&qspi,(uint16_t const *)qspi.qspiBuffer.pDataRx,
					   qspi.qspiBuffer.RxDataSize); 
	}
    
	/* release CS */
	if(message->cs_release)
	{ 
		QSPI_EndTransfer(qspi.pQspiHw);
		#if 0
		if(message->send_buf) {
			rt_uint8_t * send_ptr = &(message->send_buf[0]);
			for (i=0;i<message->length;i++)		
					rt_kprintf("QSPI Snd %x\n", send_ptr[i]);
		}

		if(message->recv_buf) {
			rt_uint8_t * recv_ptr = &(message->recv_buf[0]);
			for (i=0;i<message->length;i++)		
					rt_kprintf("QSPI Rcv %x\n", recv_ptr[i]);
		}
		#endif
	}

    return message->length;
};

void rt_hw_spi_init(void)
{
	rt_uint8_t id[3] = {0};
	static struct same70_qspi_bus same70_qspi;
	static struct rt_spi_device spi_device;
	//static const Pin pinsCS = PIN_CS;	
	static const Pin pQspipins[] = {PIN_IO0, PIN_IO1, PIN_IO2, PIN_IO3, PIN_SCK, PIN_CS};
	//PIO_Configure(&pinsCS, 1);
	PIO_Configure(pQspipins, PIO_LISTSIZE(pQspipins));
	PMC_EnablePeripheral(ID_QSPI);
	QSPI_ConfigureInterface(&qspi, QSPI_MR_SMM_SPI, QSPI_MR_CSMODE_LASTXFER | QSPI_MR_DLYCS (20));
	QSPI_ConfigureClock(qspi.pQspiHw, ClockMode_00, QSPI_SCR_SCBR(1));
	QSPI_Enable(qspi.pQspiHw);
    rt_spi_bus_register(&(same70_qspi.parent), "qspi", &same70_qspi_ops);
	rt_spi_bus_attach_device(&spi_device, "spi10", "qspi", RT_NULL);
	qspi.qspiCommand.Instruction = 0x9F;
	qspi.qspiBuffer.pDataRx = id;
	QSPI_SingleWriteSPI(&qspi, (uint16_t const *)&qspi.qspiCommand.Instruction);
	QSPI_MultiReadSPI(&qspi,  (uint16_t *)qspi.qspiBuffer.pDataRx, 3);
	QSPI_EndTransfer(qspi.pQspiHw);
	rt_kprintf("id %x %x %x\n",id[0], id[1], id[2]);
}
