#ifndef DRV_USB_H__
#define DRV_USB_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* pin device and operations for RT-Thread */
struct rt_device_usb
{
    struct rt_device parent;
};

int rt_device_usb_register(const char *name);


#ifdef __cplusplus
}
#endif

#endif

