// tlmp.h - Internal interface for the Tiny Little Mooltipass library
//
// Copyright (c) 2015 Philipp Geyer (Nistur) nistur@gmail.com
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

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
    libusb_device_handle* handle;
    void* callback;

    int state;

    unsigned char kernel;
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

void tlmpUnusedInternal(void* x);
#define tlmpUnused(x) tlmpUnusedInternal((void*)&x)

tlmpReturn tlmpAddDevice(tlmpContext* context, tlmpDevice* device);
tlmpReturn tlmpRemoveDevice(tlmpContext* context, tlmpDevice* device);
tlmpReturn tlmpFindDevice(tlmpContext* context, tlmpDevice** device, libusb_device* dev);
tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size);
tlmpReturn tlmpReceivePacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size);

tlmpReturn tlmpSendPacketAsync(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size);
tlmpReturn tlmpReceivePacketAsync(tlmpDevice* device);

#include <string.h>
#include <stdio.h>

#endif/*__TLMP_INTERNAL_H__*/
