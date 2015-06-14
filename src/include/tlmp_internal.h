#pragma once
#ifndef __TLMP_INTERNAL_H__
#define __TLMP_INTERNAL_H__

#include "tlmp.h"
#include "tlmp_messageids.h"

#include "libusb.h"

#define USB_VID (0x16D0)
#define USB_PID (0x09A0)

#define TLMP_STATE_IDLE            0x00
#define TLMP_STATE_WAITFORLOGIN    0x01
#define TLMP_STATE_WAITFORPASSWORD 0x02
#define TLMP_STATE_WAITFORSTATUS   0x03

typedef struct _tlmpNode tlmpNode;

struct _tlmpNode
{
    tlmpNode* next;
};

/***************************************
 * Library context
 * - holds current state
 ***************************************/
struct _tlmpContext
{
    libusb_context* lib;
    libusb_hotplug_callback_handle hotplug;
    tlmpNode* devices;

    tlmpConnectCallback connect;
};

struct _tlmpDevice
{
    tlmpNode header;
    libusb_device*  dev;
    void* callback;

    int state;
};

/***************************************
 * Some basic memory management wrappers
 ***************************************/
#include <stdlib.h>
#define tlmpMalloc(x) (x*)tlmpMallocInternal(sizeof(x))
#define tlmpMallocArray(x,n) (x*)tlmpMallocInternal(sizeof(x)*n)
#define tlmpFree(x)   tlmpFreeInternal((void**)&x)
void* tlmpMallocInternal(int n);
void  tlmpFreeInternal(void** p);

/***************************************
 * Error handling
 ***************************************/
extern tlmpReturn  g_tlmpError;
extern const char* g_tlmpErrors[];
#define tlmpReturn(x)				\
    {						\
	g_tlmpError = TLMP_##x;			\
	return TLMP_##x;			\
    }


tlmpReturn tlmpAddDevice(tlmpContext* context, tlmpDevice* device);
tlmpReturn tlmpRemoveDevice(tlmpContext* context, tlmpDevice* device);
tlmpReturn tlmpFindDevice(tlmpContext* context, tlmpDevice** device, libusb_device* dev);
tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size);

#include <string.h>

#endif/*__TLMP_INTERNAL_H__*/
