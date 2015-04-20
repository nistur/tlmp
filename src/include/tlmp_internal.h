#pragma once
#ifndef __TLMP_INTERNAL_H__
#define __TLMP_INTERNAL_H__

#include "tlmp.h"

#include "libusb-1.0/libusb.h"

#define USB_VID (0x16D0)
#define USB_PID (0x09A0)

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
};

struct _tlmpDevice
{
    tlmpNode header;
    libusb_device*  dev;
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

#endif/*__TLMP_INTERNAL_H__*/
