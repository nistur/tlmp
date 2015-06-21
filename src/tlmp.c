// tlmp.c - Implementation of the Tiny Little Mooltipass library
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

#include "tlmp_internal.h"
#include "sys/time.h"

// Clean up the context. Just set everything to 0.
// If any devices are connected, delete them
tlmpReturn tlmpClearContext(tlmpContext* context)
{
    tlmpNode* dev = context->devices;
    while(dev)
    {
        tlmpNode* next = dev->next;
        libusb_close(((tlmpDevice*)dev)->handle);
        tlmpFree(dev);
        dev = next;
    }
    context->devices = 0;
    
    tlmpReturn(SUCCESS);
}

// libusb callback when a mooltipass is connected or disconnected.
int tlmpHotplugCallback(libusb_context* lib, libusb_device* dev, libusb_hotplug_event evt, void* context)
{
    tlmpUnused(lib);
    tlmpContext* ctx = (tlmpContext*)context;
    switch(evt)
    {
    case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED:
    {
        tlmpDevice* device = tlmpMalloc(tlmpDevice);
        device->dev = dev;
        
        tlmpAddDevice(ctx, device);
        
    }
    break;
    case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT:
    {
        tlmpDevice* device = 0;
        if(tlmpFindDevice(ctx, &device, dev) == TLMP_SUCCESS)
            tlmpRemoveDevice(ctx, device);
        tlmpFree(device);
    }
    break;
    default:
    {
//        printf("Unhandled libusb callback 0x%X\n", evt);
    }
    }
    return 0;
}

// create a context and link up libusb to it
tlmpReturn tlmpInitContext(tlmpContext** context)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    *context = tlmpMalloc(tlmpContext);
    if(*context == 0)
        tlmpReturn(STORAGE);
    tlmpReturn clearError = tlmpClearContext(*context);
    if(clearError != TLMP_SUCCESS)
    {
        tlmpTerminateContext(context);
        return clearError;
    }

    libusb_init( &(*context)->lib );
    
    libusb_hotplug_register_callback((*context)->lib,
                                     LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                     LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                                     LIBUSB_HOTPLUG_ENUMERATE,
                                     USB_VID, USB_PID,
                                     LIBUSB_HOTPLUG_MATCH_ANY,
                                     &tlmpHotplugCallback,
                                     *context,
                                     &(*context)->hotplug);
    
    tlmpReturn(SUCCESS);
}

// Clean up the context and disconnect libusb
tlmpReturn tlmpTerminateContext(tlmpContext** context)
{
    if(*context == 0)
        tlmpReturn(NO_CONTEXT);
    
    libusb_hotplug_deregister_callback((*context)->lib,
                                       (*context)->hotplug);
    
    libusb_exit( (*context)->lib );
    
    tlmpClearContext(*context);
    
    tlmpFree(*context);
    *context = 0;
    tlmpReturn(SUCCESS);
}

// pump libusb events
tlmpReturn tlmpUpdateContext(tlmpContext* context)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));
    
    libusb_handle_events_timeout(context->lib, &tv);
    
    tlmpReturn(SUCCESS);
}

// add device to the linked list
tlmpReturn tlmpAddDevice(tlmpContext* context, tlmpDevice* device)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    
    tlmpNode** tail = &context->devices;
    while(*tail)
        tail = &(*tail)->next;
    *tail = &device->header;

    int error = libusb_open(device->dev, &device->handle); 
    if( error )
        tlmpReturn(PERMISSION);

    if(context->connect != 0)
        context->connect(context, device, 1);
    
    tlmpReturn(SUCCESS);
}

// find and remove a device if it's been disconnected
tlmpReturn tlmpRemoveDevice(tlmpContext* context, tlmpDevice* device)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    if(device == 0 || context->devices == 0)
        tlmpReturn(NO_DEVICE);

    libusb_close(device->handle);
    
    tlmpNode** node = &context->devices;
    while(*node != &device->header)
    {
        if((*node)->next)
            node = &(*node)->next;
        else
            tlmpReturn(NO_DEVICE);
    }
    
    *node = device->header.next;

    if(context->connect != 0)
        context->connect(context, device, 0);
    tlmpFree(device);

    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpFindDevice(tlmpContext* context, tlmpDevice** device, libusb_device* dev)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    if(device == 0 || dev == 0 || context->devices == 0)
        tlmpReturn(NO_DEVICE);

    tlmpNode* node = context->devices;
    while(((tlmpDevice*)node)->dev != dev)
        if(node->next != 0)
            node = node->next;
        else
            tlmpReturn(NO_DEVICE);

    *device = (tlmpDevice*)node;

    tlmpReturn(SUCCESS);
}

// Send some data to the mooltipass
tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    unsigned char buffer[64];
    size = size>62?62:size;
    buffer[0] = size;
    buffer[1] = id;
    if(size && data)
        memcpy(&buffer[2],data,size);
    size += 2;

    unsigned char hasKernel = 0;
    // This appears to be necessary in Linux to get the OS 
    // to release the standard HID driver so we can communicate
    // with the device
    if(libusb_kernel_driver_active(device->handle, 0))
    {
    	hasKernel = 1;
    	libusb_detach_kernel_driver(device->handle, 0);
    }
    
    // Let the OS know that we want to communicate with the device
    if(libusb_claim_interface(device->handle, 0))
        tlmpReturn(PERMISSION);
    
    
    int actual_size = 0;
    int error = libusb_interrupt_transfer(device->handle,
				      LIBUSB_ENDPOINT_OUT | 2,
				      buffer, size, &actual_size, 0);

    // Check that we've sent the correct data
    if(error || size != actual_size)
        tlmpReturn(IO);

    libusb_release_interface(device->handle, 0);

    // if we've disconnected a kernel driver, then let the OS
    // reattach it
    if(hasKernel)
	   libusb_attach_kernel_driver(device->handle, 0);
    
    tlmpReturn(SUCCESS);
}

// receive some data from the mooltipass
tlmpReturn tlmpReceivePacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    unsigned char buffer[64];
    unsigned char hasKernel = 0;
    // This appears to be necessary in Linux to get the OS 
    // to release the standard HID driver so we can communicate
    // with the device
    if(libusb_kernel_driver_active(device->handle, 0))
    {
    	hasKernel = 1;
    	libusb_detach_kernel_driver(device->handle, 0);
    }
    
    // Let the OS know that we want to communicate with the device
    if(libusb_claim_interface(device->handle, 0))
        tlmpReturn(PERMISSION);
    
    
    int actual_size = 0;
    int error = libusb_interrupt_transfer(device->handle,
		  		      LIBUSB_ENDPOINT_IN | 1,
				      buffer, 64, &actual_size, 0 );

    libusb_release_interface(device->handle, 0);

    // if we've disconnected a kernel driver, then let the OS
    // reattach it
    if(hasKernel)
        libusb_attach_kernel_driver(device->handle, 0);

    // Check that we've actually received what we expected to
    if(error || 
        64 != actual_size ||
        buffer[1] != id)
        tlmpReturn(IO);

    // Check that we've been given enough space to store the data
    if(size<buffer[0])
        tlmpReturn(STORAGE);
    memcpy(data, &buffer[2], size);
    
    tlmpReturn(SUCCESS);
}

// hook up an external callback for when a mooltipass has been connected or disconnected
tlmpReturn tlmpSetConnectCallback(tlmpContext* context, tlmpConnectCallback callback)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);

    context->connect = callback;
    
    tlmpReturn(SUCCESS);
}

// TODO: Async credential request
tlmpReturn tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback)
{
    tlmpUnused(domain);
    tlmpUnused(callback);
    if(device == 0)
        tlmpReturn(NO_DEVICE);
    
    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);

    //device->state = TLMP_STATE_WAITFORLOGIN;
    
    tlmpReturn(SUCCESS);
}

// TODO: Async status request
tlmpReturn tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback)
{
    tlmpUnused(callback);
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);

//    device->state = TLMP_STATE_WAITFORSTATUS;

    tlmpReturn(SUCCESS);
    
}

// Find what status the mooltipass is in
tlmpReturn tlmpGetStatus(tlmpDevice* device, tlmpStatus* status)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);

    device->state = TLMP_STATE_WAITFORSTATUS;

    tlmpReturn err =  tlmpSendPacket(device,
				     TLMP_MESSAGE_STATUS,
				     0, 0);

    // if we've went the packet, then listen for the response
    if(err == TLMP_SUCCESS)
    {
        err = tlmpReceivePacket(device,
                    TLMP_MESSAGE_STATUS,
                    status, sizeof(tlmpStatus));
    }
    device->state = TLMP_STATE_IDLE;
    return err;
    
}

// Malloc wrapper that auto-nulls
void* tlmpMallocInternal(int n)
{
    void* p = malloc(n);
    memset(p,0,n);
    return p;
}

// Safe free, checks and sets 0
void tlmpFreeInternal(void** p)
{
    if(p && *p)
    {
        free(*p);
        *p = 0;
    }      
}

// return a user-friendly error message
const char* tlmpError()
{
    return g_tlmpErrors[g_tlmpError];
}

// ignore
void tlmpUnusedInternal(void* x)
{
    x += 0;
}
