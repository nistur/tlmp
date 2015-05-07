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
    tlmpAuthCallback callback;

    int state;
};

/***************************************
 * Some basic memory management wrappers
 ***************************************/
#include <stdlib.h>
#define tlmpMalloc(x) (x*)malloc(sizeof(x))
#define tlmpFree(x)   free(x)

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
tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size);

#endif/*__TLMP_INTERNAL_H__*/
