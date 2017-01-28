#include <stdint.h>
#include <rtdevice.h>
#include "rt_board.h"
#include "board.h"
#include "drv_sdio.h"
#include "libsdmmc.h"
#include "Media.h"
#include "MEDSdcard.h"

#define SDIO_DEBUG

#ifdef SDIO_DEBUG
#define SDIO_TRACE         rt_kprintf
#else
#define SDIO_TRACE(...)
#endif /* #ifdef SDIO_DEBUG */
#define DRV_MMC 0
static struct rt_mutex lock;
static struct rt_device flash_device;
#define SECTOR_SIZE_DEFAULT 512
#define MAX_MEDS        1
sMedia medias[MAX_MEDS];
static sXdmad dmaDrv;
/** MCI driver instance. */
static sMcid mciDrv[BOARD_NUM_MCI];
/** SDCard driver instance. */
COMPILER_ALIGNED(32) sSdCard sdDrv[BOARD_NUM_MCI];


static void flash_lock(void)
{
    rt_mutex_take(&lock, RT_WAITING_FOREVER);
}

static void flash_unlock(void)
{
    rt_mutex_release(&lock);
}

/* RT-Thread device interface */
static rt_err_t sdcard_flash_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t sdcard_flash_open(rt_device_t dev, rt_uint16_t oflag)
{    
    return RT_EOK;
}

static rt_err_t sdcard_flash_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t sdcard_flash_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL) return -RT_ERROR;

        geometry->bytes_per_sector = 512;
        geometry->block_size = SD_GetBlockSize(&sdDrv[DRV_MMC]);
        if (SD_GetCardType(&sdDrv[DRV_MMC]) == CARD_TYPE_bmHC)
            geometry->sector_count = (SD_GetTotalSizeKB(&sdDrv[DRV_MMC]) + 1)  * 1024;
        else
            geometry->sector_count = SD_GetNumberBlocks(&sdDrv[DRV_MMC]);
    }
	return RT_EOK;
}

static rt_size_t sdcard_flash_read(rt_device_t dev,
                                   rt_off_t pos,
                                   void* buffer,
                                   rt_size_t size)
{
   
    flash_lock();
   	unsigned char result;
	unsigned int addr, len; 
	if (medias[DRV_MMC].blockSize < SECTOR_SIZE_DEFAULT)    
	{        
		addr = pos * (SECTOR_SIZE_DEFAULT / medias[DRV_MMC].blockSize);        
		len  = size * (SECTOR_SIZE_DEFAULT / medias[DRV_MMC].blockSize);    
	}    
	else    
	{        
		addr = pos;        
		len  = size;    
	}    
	result = MED_Read(&medias[DRV_MMC], addr, (void*)buffer, len, NULL, NULL);    
	if( result != MED_STATUS_SUCCESS )
	{        
		SDIO_TRACE("MED_Read pb: 0x%X\n\r", result);
	}
    flash_unlock();
    if(result == MED_STATUS_SUCCESS)
    	return size;

	return 0;
}

static rt_size_t sdcard_flash_write(rt_device_t dev,
                                    rt_off_t pos,
                                    const void* buffer,
                                    rt_size_t size)
{ 
 
    flash_lock();
	unsigned int result;	 
	void * tmp;	
	tmp = (void *) buffer;    
	unsigned int addr, len;	
	if (medias[DRV_MMC].blockSize < SECTOR_SIZE_DEFAULT)    
	{        
		addr = pos * (SECTOR_SIZE_DEFAULT / medias[DRV_MMC].blockSize);        
		len  = size * (SECTOR_SIZE_DEFAULT / medias[DRV_MMC].blockSize);    
	}    
	else    
	{        
		addr = pos;        
		len  = size;    
	}    
	result = MED_Write(&medias[DRV_MMC], addr, (void*)tmp, len, NULL, NULL);    
	if( result != MED_STATUS_SUCCESS )    
	{        
		SDIO_TRACE("MED_Write pb: 0x%X\n\r", result);
	}    
    flash_unlock();

	if(result == MED_STATUS_SUCCESS)
    	return size;

	return 0;
}
void XDMAC_Handler(void)
{	
	rt_interrupt_enter();
	XDMAD_Handler(&dmaDrv);
	rt_interrupt_leave();
}
/** * MCI interrupt handler. Forwards the event to the MCI driver handlers. */
void HSMCI_Handler(void)
{	
	rt_interrupt_enter();
	MCID_Handler(&mciDrv[0]);
	rt_interrupt_leave();
}

rt_int32_t rt_hw_sdio_init(void)
{
	rt_int32_t stat = RT_EOK;
	uint32_t i;	
	static const Pin pinsSd[] = {BOARD_MCI_PINS_SLOTA, BOARD_MCI_PIN_CK};
	static const Pin pinsCd[] = {BOARD_MCI_PIN_CD};	
	PIO_Configure(pinsSd, PIO_LISTSIZE(pinsSd));	
	PIO_Configure(pinsCd, PIO_LISTSIZE(pinsCd));
	XDMAD_Initialize(&dmaDrv, 0);	
	NVIC_ClearPendingIRQ(XDMAC_IRQn);	
	NVIC_SetPriority(XDMAC_IRQn, 1);	
	NVIC_EnableIRQ(XDMAC_IRQn);	
	MCID_Init(&mciDrv[0], HSMCI, ID_HSMCI, BOARD_MCK, &dmaDrv, 0);	
	NVIC_ClearPendingIRQ(HSMCI_IRQn);	
	NVIC_SetPriority(HSMCI_IRQn, 3);	
	NVIC_EnableIRQ(HSMCI_IRQn);	
	for (i = 0; i < BOARD_NUM_MCI; i++)		
		SDD_InitializeSdmmcMode(&sdDrv[i], &mciDrv[i], 0);
	rt_uint8_t error = SD_Init(&sdDrv[DRV_MMC]);
	if (error != SDMMC_OK)
		return -RT_ERROR;
	if(!(MEDSdcard_Initialize(&medias[DRV_MMC], &sdDrv[DRV_MMC])))            
	{              
		SDIO_TRACE(" SD Init fail\n\r ");              
		stat = RT_ERROR;            
	}
    /* initialize mutex */
    if (rt_mutex_init(&lock, "sd_mutex", RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        rt_kprintf("init sd lock mutex failed\n");
        return -RT_ENOSYS;
    }
    
    /* register device */
    flash_device.type    = RT_Device_Class_Block;
    flash_device.init    = sdcard_flash_init;
    flash_device.open    = sdcard_flash_open;
    flash_device.close   = sdcard_flash_close;
    flash_device.read    = sdcard_flash_read;
    flash_device.write   = sdcard_flash_write;
    flash_device.control = sdcard_flash_control;
    /* no private */
    flash_device.user_data = RT_NULL;

    rt_device_register(&flash_device, "sd0",
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

    return stat;
}
