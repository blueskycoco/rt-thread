/*
 * File      : same70_sdio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author		Notes
 * 2011-07-25     weety		first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <drivers/mmcsd_core.h>
#include "board.h"
#include "hsmci.h"
#include "drv_sdio.h"


//#define USE_SLOT_B
#define RT_MCI_DBG

#ifdef RT_MCI_DBG
#define mci_dbg(fmt, ...)  rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define mci_dbg(fmt, ...)
#endif


#define REQ_ST_INIT	(1U << 0)
#define REQ_ST_CMD	(1U << 1)
#define REQ_ST_STOP	(1U << 2)

struct same70_sdio {
	struct rt_mmcsd_host *host;
	struct rt_mmcsd_req *req;
	struct rt_mmcsd_cmd *cmd;
	struct rt_timer timer;
	//struct rt_semaphore sem_ack;
	rt_uint32_t *buf;
	rt_uint32_t current_status;
};
static sXdmad dmaDrv;

/*
 * Reset the controller and restore most of the state
 */
static void at91_reset_host()
{
	rt_uint32_t level;
	rt_kprintf("in at91_reset_host\r\n");

	level = rt_hw_interrupt_disable();
	/* Disable */
	HSMCI_Disable(HSMCI);
	/* MR reset */
	HSMCI_ConfigureMode(HSMCI, HSMCI_GetMode(HSMCI) & (HSMCI_MR_CLKDIV_Msk
					| HSMCI_MR_PWSDIV_Msk));
	/* BLKR reset */
	HSMCI_ConfigureTransfer(HSMCI, 0, 0);

	/* Cancel ... */
	HSMCI_Reset(HSMCI, 1);

	rt_hw_interrupt_enable(level);

}



static void at91_timeout_timer(void *data)
{
	struct same70_sdio *mci;

	mci = (struct same70_sdio *)data;

	if (mci->req) 
	{
		rt_kprintf("Timeout waiting end of packet\n");

		if (mci->current_status == REQ_ST_CMD) 
		{
			if (mci->req->cmd && mci->req->data) 
			{
				mci->req->data->err = -RT_ETIMEOUT;
			} 
			else 
			{
				if (mci->req->cmd)
					mci->req->cmd->err = -RT_ETIMEOUT;
			}
		}
		else if (mci->current_status == REQ_ST_STOP) 
		{
			mci->req->stop->err = -RT_ETIMEOUT;
		}

		at91_reset_host();
		mmcsd_req_complete(mci->host);
	}
}

/*
 * Process the next step in the request
 */
static void same70_sdio_process_next(struct same70_sdio *mci)
{
	if (mci->current_status == REQ_ST_INIT) 
	{
		mci->current_status = REQ_ST_CMD;
		same70_sdio_send_command(mci, mci->req->cmd);
	}
	else if ((mci->current_status == REQ_ST_CMD) && mci->req->stop) 
	{
		mci->current_status = REQ_ST_STOP;
		same70_sdio_send_command(mci, mci->req->stop);
	} 
	else 
	{
		rt_timer_stop(&mci->timer);
		/* the mci controller hangs after some transfers,
		 * and the workaround is to reset it after each transfer.
		 */
		at91_reset_host();
		mmcsd_req_complete(mci->host);
	}
}

/*
 * Handle an MMC request
 */
static void same70_sdio_request(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
#if 0
	rt_uint32_t timeout = RT_TICK_PER_SECOND;
	struct same70_sdio *mci = host->private_data;
	mci->req = req;
	mci->current_status = REQ_ST_INIT;

	rt_timer_control(&mci->timer, RT_TIMER_CTRL_SET_TIME, (void*)&timeout);
	rt_timer_start(&mci->timer);

	same70_sdio_process_next(mci);
#endif
}
void XDMAC_Handler(void)
{	
	rt_kprintf("in XDMAC_Handler\r\n");
	XDMAD_Handler(&dmaDrv);
}

/*
 * Handle an interrupt
 */
void HSMCI_Handler(void)
{
	rt_kprintf("in HSMCI_Handler\r\n");
#if 0
	//MCID_Handler(&mciDrv[0]);
	struct same70_sdio *mci = (struct same70_sdio *)param;
	rt_int32_t completed = 0;
	rt_uint32_t int_status, int_mask;

	int_status = same70_sdio_read(same70_sdio_SR);
	int_mask = same70_sdio_read(same70_sdio_IMR);

	mci_dbg("MCI irq: status = %08X, %08X, %08X\n", int_status, int_mask,
		int_status & int_mask);

	int_status = int_status & int_mask;

	if (int_status & same70_sdio_ERRORS) 
	{
		completed = 1;

		if (int_status & same70_sdio_UNRE)
			mci_dbg("MMC: Underrun error\n");
		if (int_status & same70_sdio_OVRE)
			mci_dbg("MMC: Overrun error\n");
		if (int_status & same70_sdio_DTOE)
			mci_dbg("MMC: Data timeout\n");
		if (int_status & same70_sdio_DCRCE)
			mci_dbg("MMC: CRC error in data\n");
		if (int_status & same70_sdio_RTOE)
			mci_dbg("MMC: Response timeout\n");
		if (int_status & same70_sdio_RENDE)
			mci_dbg("MMC: Response end bit error\n");
		if (int_status & same70_sdio_RCRCE)
			mci_dbg("MMC: Response CRC error\n");
		if (int_status & same70_sdio_RDIRE)
			mci_dbg("MMC: Response direction error\n");
		if (int_status & same70_sdio_RINDE)
			mci_dbg("MMC: Response index error\n");
	} 
	else 
	{
		/* Only continue processing if no errors */

		if (int_status & same70_sdio_TXBUFE) 
		{
			mci_dbg("TX buffer empty\n");
			same70_sdio_handle_transmitted(mci);
		}

		if (int_status & same70_sdio_ENDRX) 
		{
			mci_dbg("ENDRX\n");
			same70_sdio_post_dma_read(mci);
		}

		if (int_status & same70_sdio_RXBUFF) 
		{
			mci_dbg("RX buffer full\n");
			same70_sdio_write(AT91_PDC_PTCR, AT91_PDC_RXTDIS | AT91_PDC_TXTDIS);
			same70_sdio_write(same70_sdio_IDR, same70_sdio_RXBUFF | same70_sdio_ENDRX);
			completed = 1;
		}

		if (int_status & same70_sdio_ENDTX)
			mci_dbg("Transmit has ended\n");

		if (int_status & same70_sdio_NOTBUSY) 
		{
			mci_dbg("Card is ready\n");
			//same70_sdio_update_bytes_xfered(host);
			completed = 1;
		}

		if (int_status & same70_sdio_DTIP)
			mci_dbg("Data transfer in progress\n");

		if (int_status & same70_sdio_BLKE) 
		{
			mci_dbg("Block transfer has ended\n");
			if (mci->req->data && mci->req->data->blks > 1) 
			{
				/* multi block write : complete multi write
				 * command and send stop */
				completed = 1;
			} 
			else 
			{
				same70_sdio_write(same70_sdio_IER, same70_sdio_NOTBUSY);
			}
		}

		/*if (int_status & same70_sdio_SDIOIRQA)
			rt_mmcsd_signal_sdio_irq(host->mmc);*/

		if (int_status & same70_sdio_SDIOIRQB)
			sdio_irq_wakeup(mci->host);

		if (int_status & same70_sdio_TXRDY)
			mci_dbg("Ready to transmit\n");

		if (int_status & same70_sdio_RXRDY)
			mci_dbg("Ready to receive\n");

		if (int_status & same70_sdio_CMDRDY) 
		{
			mci_dbg("Command ready\n");
			completed = same70_sdio_handle_cmdrdy(mci);
		}
	}

	if (completed) 
	{
		mci_dbg("Completed command\n");
		same70_sdio_write(same70_sdio_IDR, 0xffffffff & ~(same70_sdio_SDIOIRQA | same70_sdio_SDIOIRQB));
		same70_sdio_completed_command(mci, int_status);
	} 
	else
		same70_sdio_write(same70_sdio_IDR, int_status & ~(same70_sdio_SDIOIRQA | same70_sdio_SDIOIRQB));
#endif
}


/*
 * Set the IOCFG
 */
static void same70_sdio_set_iocfg(struct rt_mmcsd_host *host, struct rt_mmcsd_io_cfg *io_cfg)
{
	rt_uint32_t clkdiv;

	rt_kprintf ("sdio_iocfg clock %ld, bus_width %d, power_mode %d\r\n",
		io_cfg->clock, io_cfg->bus_width, io_cfg->power_mode);
	if (io_cfg->clock == 0) 
	{
		/* Disable the MCI controller */
		HSMCI_Disable (HSMCI);
		clkdiv = 0;
	}
	else 
	{
		/* Enable the MCI controller */
		HSMCI_Enable(HSMCI);

		if ((BOARD_MCK % io_cfg->clock) == 0)
			clkdiv = BOARD_MCK / io_cfg->clock;
		else
			clkdiv = ((BOARD_MCK + io_cfg->clock) / io_cfg->clock);		
		
		/* Modify MR */
		HSMCI_DivCtrl(HSMCI, clkdiv, 0x7);

		mci_dbg("clkdiv = %d. \n", clkdiv);
	}
	if (io_cfg->bus_width == MMCSD_BUS_WIDTH_4) 
	{
		mci_dbg("MMC: Setting controller bus width to 4\n");
		HSMCI_SetBusWidth(HSMCI, 4);
	}
	else 
	{
		mci_dbg("MMC: Setting controller bus width to 1\n");
		HSMCI_SetBusWidth(HSMCI, 1);
	}

	/* maybe switch power to the card */
	switch (io_cfg->power_mode) 
	{
		case MMCSD_POWER_OFF:
			break;
		case MMCSD_POWER_UP:
			break;
		case MMCSD_POWER_ON:
			/*same70_sdio_write(same70_sdio_ARGR, 0);
			same70_sdio_write(same70_sdio_CMDR, 0|same70_sdio_SPCMD_INIT|same70_sdio_OPDCMD);
			mci_dbg("MCI_SR=0x%08x\n", same70_sdio_read(same70_sdio_SR));
			while (!(same70_sdio_read(same70_sdio_SR) & same70_sdio_CMDRDY)) 
			{
				
			}
			mci_dbg("at91 mci power on\n");*/
			break;
		default:
			rt_kprintf("unknown power_mode %d\n", io_cfg->power_mode);
			break;
	}

}


static void same70_sdio_enable_sdio_irq(struct rt_mmcsd_host *host, rt_int32_t enable)
{
	if (enable)
	{
		HSMCI_EnableIt(HSMCI, 0xFFFFFFFF);
	}
	else
	{
		HSMCI_DisableIt(HSMCI, 0xFFFFFFFF);
	}
}


static const struct rt_mmcsd_host_ops ops = {
	same70_sdio_request,
	same70_sdio_set_iocfg,
        RT_NULL,
	same70_sdio_enable_sdio_irq,
};

static void mci_gpio_init()
{
	static const Pin pinsSd[] = {BOARD_MCI_PINS_SLOTA, BOARD_MCI_PIN_CK};
	static const Pin pinsCd[] = {BOARD_MCI_PIN_CD};
	
	PIO_Configure(pinsSd, PIO_LISTSIZE(pinsSd));	
	PIO_Configure(pinsCd, PIO_LISTSIZE(pinsCd));
	if(PIO_Get(&pinsCd[0]))
		rt_kprintf("cd is protected\n");
}

rt_int32_t rt_hw_sdio_init(void)
{
	struct rt_mmcsd_host *host;
	struct same70_sdio *mci;

	host = mmcsd_alloc_host();
	if (!host) 
	{
		return -RT_ERROR;
	}

	mci = rt_malloc(sizeof(struct same70_sdio));
	if (!mci) 
	{
		rt_kprintf("alloc mci failed\n");
		goto err;
	}

	rt_memset(mci, 0, sizeof(struct same70_sdio));

	host->ops = &ops;
	host->freq_min = 375000;
	host->freq_max = 25000000;
	host->valid_ocr = VDD_32_33 | VDD_33_34;
	host->flags = MMCSD_BUSWIDTH_4 | MMCSD_MUTBLKWRITE | \
				MMCSD_SUP_HIGHSPEED | MMCSD_SUP_SDIO_IRQ;
	host->max_seg_size = 65535;
	host->max_dma_segs = 2;
	host->max_blk_size = 512;
	host->max_blk_count = 4096;

	mci->host = host;
	XDMAD_Initialize(&dmaDrv, 0);
	NVIC_ClearPendingIRQ(XDMAC_IRQn);	
	NVIC_SetPriority(XDMAC_IRQn, 1);	
	NVIC_EnableIRQ(XDMAC_IRQn);

	mci_gpio_init();
	PMC_EnablePeripheral(ID_HSMCI);
	
	HSMCI_Reset(HSMCI,0);
	HSMCI_Disable (HSMCI);
	HSMCI_DisableIt(HSMCI, 0xFFFFFFFF);
	HSMCI_ConfigureDataTO(HSMCI, HSMCI_DTOR_DTOCYC(0xFF)
						   | HSMCI_DTOR_DTOMUL_1048576);
	HSMCI_ConfigureCompletionTO(HSMCI , HSMCI_CSTOR_CSTOCYC(0xFF)
								 | HSMCI_CSTOR_CSTOMUL_1048576);
	/* Set the Mode Register: 400KHz */
	uint16_t clkDiv = (BOARD_MCK / (MCI_INITIAL_SPEED << 1)) - 1;
	HSMCI_ConfigureMode(HSMCI, (clkDiv | HSMCI_MR_PWSDIV(0x7)));

	HSMCI_Enable(HSMCI);
	HSMCI_Configure(HSMCI, HSMCI_CFG_FIFOMODE | HSMCI_CFG_FERRCTRL);
	/* Enable DMA */
	HSMCI_EnableDma(HSMCI, 1);
	NVIC_ClearPendingIRQ(HSMCI_IRQn);	
	NVIC_SetPriority(HSMCI_IRQn, 3);	
	NVIC_EnableIRQ(HSMCI_IRQn);

	rt_timer_init(&mci->timer, "mci_timer", 
		at91_timeout_timer, 
		mci, 
		RT_TICK_PER_SECOND, 
		RT_TIMER_FLAG_PERIODIC);

	host->private_data = mci;

	mmcsd_change(host);

	return 0;

err:
	mmcsd_free_host(host);

	return -RT_ENOMEM;
}

