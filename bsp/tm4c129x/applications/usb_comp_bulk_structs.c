//*****************************************************************************
//
// usb_serial_structs.c - Data structures defining this CDC USB device.
//
// Copyright (c) 2009-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdbulk.h"
#include "usblib/device/usbdcomp.h"
#include "usb_comp_bulk_structs.h"

//*****************************************************************************
//
// The languages supported by this device.
//
//*****************************************************************************
const uint8_t g_pui8LangDescriptor[] =
{
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

//*****************************************************************************
//
// The manufacturer string.
//
//*****************************************************************************
const uint8_t g_pui8ManufacturerString[] =
{
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

//*****************************************************************************
//
// The product string.
//
//*****************************************************************************
const uint8_t g_pui8ProductString[] =
{
    2 + (16 * 2),
    USB_DTYPE_STRING,
    'T', 0, 'M', 0, '4', 0, 'C', 0, '1', 0, '2', 0, '9', 0, ' ', 0,
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0
};

//*****************************************************************************
//
// The serial number string.
//
//*****************************************************************************
const uint8_t g_pui8SerialNumberString[] =
{
    2 + (8 * 2),
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

//*****************************************************************************
//
// The control interface description string.
//
//*****************************************************************************
const uint8_t g_pui8ControlInterfaceString[] =
{
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'A', 0, 'C', 0, 'M', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,
    'r', 0, 'o', 0, 'l', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

//*****************************************************************************
//
// The configuration description string.
//
//*****************************************************************************
const uint8_t g_pui8ConfigString[] =
{
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

//*****************************************************************************
//
// The descriptor string table.
//
//*****************************************************************************
const uint8_t * const g_pui8StringDescriptors[] =
{
    g_pui8LangDescriptor,
    g_pui8ManufacturerString,
    g_pui8ProductString,
    g_pui8SerialNumberString,
    g_pui8ControlInterfaceString,
    g_pui8ConfigString
};

#define NUM_STRING_DESCRIPTORS (sizeof(g_pui8StringDescriptors) /             \
                                sizeof(uint8_t *))

//*****************************************************************************
//
// The CDC device initialization and customization structures. In this case,
// we are using USBBuffers between the CDC device class driver and the
// application code. The function pointers and callback data values are set
// to insert a buffer in each of the data channels, transmit and receive.
//
// With the buffer in place, the CDC channel callback is set to the relevant
// channel function and the callback data is set to point to the channel
// instance data. The buffer, in turn, has its callback set to the application
// function and the callback data set to our CDC instance structure.
//
//*****************************************************************************
tUSBDBulkDevice g_psBULKDevice[NUM_BULK_DEVICES] =
{
    {
	    USB_VID_TI_1CBE,
	    USB_PID_BULK,
	    500,
	    USB_CONF_ATTR_SELF_PWR,
	    USBBufferEventCallback,
	    (void *)&g_sRxBuffer[0],
	    USBBufferEventCallback,
	    (void *)&g_sTxBuffer[0],
	    g_pui8StringDescriptors,
	    NUM_STRING_DESCRIPTORS
	},
    {
	    USB_VID_TI_1CBE,
	    USB_PID_BULK,
	    500,
	    USB_CONF_ATTR_SELF_PWR,
	    USBBufferEventCallback,
	    (void *)&g_sRxBuffer[1],
	    USBBufferEventCallback,
	    (void *)&g_sTxBuffer[1],
	    g_pui8StringDescriptors,
	    NUM_STRING_DESCRIPTORS
	},	
	{
		USB_VID_TI_1CBE,
		USB_PID_BULK,
		500,
		USB_CONF_ATTR_SELF_PWR,
		USBBufferEventCallback,
		(void *)&g_sRxBuffer[2],
		USBBufferEventCallback,
		(void *)&g_sTxBuffer[2],
		g_pui8StringDescriptors,
		NUM_STRING_DESCRIPTORS
	},
	{
		USB_VID_TI_1CBE,
		USB_PID_BULK,
		500,
		USB_CONF_ATTR_SELF_PWR,
		USBBufferEventCallback,
		(void *)&g_sRxBuffer[3],
		USBBufferEventCallback,
		(void *)&g_sTxBuffer[3],
		g_pui8StringDescriptors,
		NUM_STRING_DESCRIPTORS
	},
	{
		USB_VID_TI_1CBE,
		USB_PID_BULK,
		500,
		USB_CONF_ATTR_SELF_PWR,
		USBBufferEventCallback,
		(void *)&g_sRxBuffer[4],
		USBBufferEventCallback,
		(void *)&g_sTxBuffer[4],
		g_pui8StringDescriptors,
		NUM_STRING_DESCRIPTORS
	}
};

//*****************************************************************************
//
// Receive buffer (from the USB perspective).
//
//*****************************************************************************
uint8_t g_ppui8USBRxBuffer[NUM_BULK_DEVICES][UART_BUFFER_SIZE];
uint8_t g_ppui8RxBufferWorkspace[NUM_BULK_DEVICES][USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer g_sRxBuffer[NUM_BULK_DEVICES] =
{
    {
        false,                          // This is a receive buffer.
        RxHandlerBulk,                  // pfnCallback
        (void *)&g_psBULKDevice[0],      // Callback data is our device pointer.
        USBDBulkPacketRead,              // pfnTransfer
        USBDBulkRxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[0],      // pvHandle
        g_ppui8USBRxBuffer[0],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
        g_ppui8RxBufferWorkspace[0]     // pvWorkspace
    },
    {
        false,                          // This is a receive buffer.
        RxHandlerBulk,                   // pfnCallback
        (void *)&g_psBULKDevice[1],      // Callback data is our device pointer.
        USBDBulkPacketRead,              // pfnTransfer
        USBDBulkRxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[1],      // pvHandle
        g_ppui8USBRxBuffer[1],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
        g_ppui8RxBufferWorkspace[1]     // pvWorkspace
    },
    {
        false,                          // This is a receive buffer.
        RxHandlerBulk,                   // pfnCallback
        (void *)&g_psBULKDevice[2],      // Callback data is our device pointer.
        USBDBulkPacketRead,              // pfnTransfer
        USBDBulkRxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[2],      // pvHandle
        g_ppui8USBRxBuffer[2],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
        g_ppui8RxBufferWorkspace[2]     // pvWorkspace
    },
    {
        false,                          // This is a receive buffer.
        RxHandlerBulk,                   // pfnCallback
        (void *)&g_psBULKDevice[3],      // Callback data is our device pointer.
        USBDBulkPacketRead,              // pfnTransfer
        USBDBulkRxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[3],      // pvHandle
        g_ppui8USBRxBuffer[3],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
        g_ppui8RxBufferWorkspace[3]     // pvWorkspace
    },
    {
        false,                          // This is a receive buffer.
        RxHandlerBulk,                   // pfnCallback
        (void *)&g_psBULKDevice[4],      // Callback data is our device pointer.
        USBDBulkPacketRead,              // pfnTransfer
        USBDBulkRxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[4],      // pvHandle
        g_ppui8USBRxBuffer[4],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
        g_ppui8RxBufferWorkspace[4]     // pvWorkspace
    }
};

//*****************************************************************************
//
// Transmit buffer (from the USB perspective).
//
//*****************************************************************************
uint8_t g_ppcUSBTxBuffer[NUM_BULK_DEVICES][UART_BUFFER_SIZE_TX];
uint8_t g_ppucTxBufferWorkspace[NUM_BULK_DEVICES][USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer g_sTxBuffer[NUM_BULK_DEVICES] =
{
    {
        true,                           // This is a transmit buffer.
        TxHandlerBulk,                      // pfnCallback
        (void *)&g_psBULKDevice[0],      // Callback data is our device pointer.
        USBDBulkPacketWrite,             // pfnTransfer
        USBDBulkTxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[0],      // pvHandle
        g_ppcUSBTxBuffer[0],            // pcBuffer
        UART_BUFFER_SIZE_TX,               // ulBufferSize
        g_ppucTxBufferWorkspace[0]      // pvWorkspace
    },
    {
        true,                           // This is a transmit buffer.
        TxHandlerBulk,                   // pfnCallback
        (void *)&g_psBULKDevice[1],      // Callback data is our device pointer.
        USBDBulkPacketWrite,             // pfnTransfer
        USBDBulkTxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[1],      // pvHandle
        g_ppcUSBTxBuffer[1],            // pcBuffer
        UART_BUFFER_SIZE_TX,               // ulBufferSize
        g_ppucTxBufferWorkspace[1]      // pvWorkspace
    },
    {
        true,                           // This is a transmit buffer.
        TxHandlerBulk,                      // pfnCallback
        (void *)&g_psBULKDevice[2],      // Callback data is our device pointer.
        USBDBulkPacketWrite,             // pfnTransfer
        USBDBulkTxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[2],      // pvHandle
        g_ppcUSBTxBuffer[2],            // pcBuffer
        UART_BUFFER_SIZE_TX,               // ulBufferSize
        g_ppucTxBufferWorkspace[2]      // pvWorkspace
    },
    {
        true,                           // This is a transmit buffer.
        TxHandlerBulk,                      // pfnCallback
        (void *)&g_psBULKDevice[3],      // Callback data is our device pointer.
        USBDBulkPacketWrite,             // pfnTransfer
        USBDBulkTxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[3],      // pvHandle
        g_ppcUSBTxBuffer[3],            // pcBuffer
        UART_BUFFER_SIZE_TX,               // ulBufferSize
        g_ppucTxBufferWorkspace[3]      // pvWorkspace
    },
    {
        true,                           // This is a transmit buffer.
        TxHandlerBulk,                      // pfnCallback
        (void *)&g_psBULKDevice[4],      // Callback data is our device pointer.
        USBDBulkPacketWrite,             // pfnTransfer
        USBDBulkTxPacketAvailable,       // pfnAvailable
        (void *)&g_psBULKDevice[4],      // pvHandle
        g_ppcUSBTxBuffer[4],            // pcBuffer
        UART_BUFFER_SIZE_TX,               // ulBufferSize
        g_ppucTxBufferWorkspace[4]      // pvWorkspace
    }
};

//****************************************************************************
//
// The memory allocated to hold the composite descriptor that is created by
// the call to USBDCompositeInit().
//
//****************************************************************************
uint8_t g_pui8DescriptorData[DESCRIPTOR_DATA_SIZE];

tCompositeEntry g_psCompEntries[NUM_BULK_DEVICES];

//****************************************************************************
//
// Allocate the Device Data for the top level composite device class.
//
//****************************************************************************
tUSBDCompositeDevice g_sCompDevice =
{
    //
    // Stellaris VID.
    //
    USB_VID_TI_1CBE,

    //
    // Stellaris PID for composite serial device.
    //
    /*USB_PID_COMP_SERIAL,*/USB_PID_BULK,

    //
    // This is in 2mA increments so 500mA.
    //
    250,

    //
    // Bus powered device.
    //
    USB_CONF_ATTR_BUS_PWR,

    //
    // There is no need for a default composite event handler.
    //
    0,

    //
    // The string table.
    //
    g_pui8StringDescriptors,
    NUM_STRING_DESCRIPTORS,

    //
    // The Composite device array.
    //
    NUM_BULK_DEVICES,
    g_psCompEntries
};
