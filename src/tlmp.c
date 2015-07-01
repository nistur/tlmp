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
#ifndef WIN32
# include "sys/time.h"
#endif

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
    
    tlmpReturnCode(SUCCESS);
}

// libusb callback when a mooltipass is connected or disconnected.
int tlmpHotplugCallback(libusb_context* lib, libusb_device* dev, libusb_hotplug_event evt, void* context)
{
	tlmpContext* ctx = (tlmpContext*)context;
    tlmpUnused(lib);
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
    tlmpReturn clearError = TLMP_SUCCESS;
	if(context == 0)
        tlmpReturnCode(NO_CONTEXT);
    *context = tlmpMalloc(tlmpContext);
    if(*context == 0)
        tlmpReturnCode(STORAGE);
    clearError = tlmpClearContext(*context);
    if(clearError != TLMP_SUCCESS)
    {
        tlmpTerminateContext(context);
        return clearError;
    }

    libusb_init( &(*context)->lib );
    
	if(libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
		libusb_hotplug_register_callback((*context)->lib,
		LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
		LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
		LIBUSB_HOTPLUG_ENUMERATE,
		USB_VID, USB_PID,
		LIBUSB_HOTPLUG_MATCH_ANY,
		(libusb_hotplug_callback_fn)&tlmpHotplugCallback,
		*context,
		&(*context)->hotplug);
    
    tlmpReturnCode(SUCCESS);
}

// Clean up the context and disconnect libusb
tlmpReturn tlmpTerminateContext(tlmpContext** context)
{
    if(*context == 0)
        tlmpReturnCode(NO_CONTEXT);
    
    libusb_hotplug_deregister_callback((*context)->lib,
                                       (*context)->hotplug);
    
    tlmpClearContext(*context);

    libusb_exit( (*context)->lib );
    
    tlmpFree(*context);
    *context = 0;
    tlmpReturnCode(SUCCESS);
}

// pump libusb events
tlmpReturn tlmpUpdateContext(tlmpContext* context)
{
	struct timeval tv;
    if(context == 0)
        tlmpReturnCode(NO_CONTEXT);
    
    memset(&tv, 0, sizeof(tv));
    
    libusb_handle_events_timeout(context->lib, &tv);
    
    tlmpReturnCode(SUCCESS);
}

// add device to the linked list
tlmpReturn tlmpAddDevice(tlmpContext* context, tlmpDevice* device)
{
	int error = 0;
    if(context == 0)
        tlmpReturnCode(NO_CONTEXT);

    printf("Adding device\n");
	{
		tlmpNode** tail = &context->devices;
		while(*tail)
			tail = &(*tail)->next;
		*tail = &device->header;
	}
    error = libusb_open(device->dev, &device->handle); 
    if( error )
        tlmpReturnCode(PERMISSION);

    if(context->connect != 0)
        context->connect(context, device, 1);
    
    tlmpReturnCode(SUCCESS);
}

// find and remove a device if it's been disconnected
tlmpReturn tlmpRemoveDevice(tlmpContext* context, tlmpDevice* device)
{
    if(context == 0)
        tlmpReturnCode(NO_CONTEXT);
    if(device == 0 || context->devices == 0)
        tlmpReturnCode(NO_DEVICE);

    libusb_close(device->handle);
    
	{
		tlmpNode** node = &context->devices;
		while(*node != &device->header)
		{
			if((*node)->next)
				node = &(*node)->next;
			else
				tlmpReturnCode(NO_DEVICE);
		}

		*node = device->header.next;
	}

    if(context->connect != 0)
        context->connect(context, device, 0);
    tlmpFree(device);

    tlmpReturnCode(SUCCESS);
}

tlmpReturn tlmpFindDevice(tlmpContext* context, tlmpDevice** device, libusb_device* dev)
{
    if(context == 0)
        tlmpReturnCode(NO_CONTEXT);
    if(device == 0 || dev == 0 || context->devices == 0)
        tlmpReturnCode(NO_DEVICE);

	{
		tlmpNode* node = context->devices;
		while(((tlmpDevice*)node)->dev != dev)
			if(node->next != 0)
				node = node->next;
			else
				tlmpReturnCode(NO_DEVICE);

		*device = (tlmpDevice*)node;
	}

    tlmpReturnCode(SUCCESS);
}

// Send some data to the mooltipass
tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);

	{
		unsigned char buffer[64];
		unsigned char hasKernel = 0;
		int actual_size = 0;
		int error = 0;

		size = size>62?62:size;
		buffer[0] = size;
		buffer[1] = id;
		if(size && data)
			memcpy(&buffer[2],data,size);
		size += 2;

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
			tlmpReturnCode(PERMISSION);
    
    
		error = libusb_interrupt_transfer(device->handle,
						  LIBUSB_ENDPOINT_OUT | 2,
						  buffer, size, &actual_size, 0);

		// Check that we've sent the correct data
		if(error || size != actual_size)
			tlmpReturnCode(IO);

		libusb_release_interface(device->handle, 0);

		// if we've disconnected a kernel driver, then let the OS
		// reattach it
		if(hasKernel)
		   libusb_attach_kernel_driver(device->handle, 0);
	}
    tlmpReturnCode(SUCCESS);
}

// receive some data from the mooltipass
tlmpReturn tlmpReceivePacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);

	{
		unsigned char buffer[64];
		unsigned char hasKernel = 0;
		int actual_size = 0;
		int error = 0;

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
			tlmpReturnCode(PERMISSION);
    
    
		error = libusb_interrupt_transfer(device->handle,
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
			tlmpReturnCode(IO);

		// Check that we've been given enough space to store the data
		if(size<buffer[0])
			tlmpReturnCode(STORAGE);
		memcpy(data, &buffer[2], size);
	}
    tlmpReturnCode(SUCCESS);
}

// Callback from libusb when a packet got sent to the mooltipass
void tlmpPacketSent(struct libusb_transfer* transfer)
{
    tlmpDevice* device = (tlmpDevice*)transfer->user_data;
    tlmpFree(transfer->buffer);
    libusb_free_transfer(transfer);

    tlmpReceivePacketAsync(device);
}

// Callback from libusb when a packet got received from the mooltipass
void tlmpPacketReceived(struct libusb_transfer* transfer)
{
    tlmpDevice* device = (tlmpDevice*)transfer->user_data;
    unsigned char* buffer = transfer->buffer;
    libusb_free_transfer(transfer);

    if(device->kernel)
	libusb_attach_kernel_driver(device->handle, 0);

	{
		unsigned char size = buffer[0];
		unsigned char id = buffer[1];

		// Check that we were expecting this packet
		if(device->state == id)
		{
		// Handle the packet based on the type of packet it is
		if(id == TLMP_MESSAGE_STATUS &&
		   size == 1)
		{
			if(device->callback)
			((tlmpStatusCallback)device->callback)(device, buffer[2]);
			device->state = TLMP_STATE_IDLE;
		}
		// Mooltipass set the current context
		else if(id == TLMP_MESSAGE_SETCONTEXT &&
		   size == 1)
		{
			// Mooltipass returns 0x01 on sucessfully found context
			if(buffer[2] == 0x01)
			{
			tlmpSendPacketAsync(device, TLMP_MESSAGE_GETLOGIN, 0, 0);
			}
			else
			{
			device->state = TLMP_STATE_IDLE;
			}
		}
		// Mooltipass returned login for the current context
		else if(id == TLMP_MESSAGE_GETLOGIN &&
			size > 1)
		{
			tlmpFree(device->user);
			device->user = tlmpMallocArray(unsigned char, 63);
			memcpy(device->user, &buffer[2], size + 1);

			tlmpSendPacketAsync(device, TLMP_MESSAGE_GETPASSWORD, 0, 0);
			
		}
		// Mooltipass returned a password for the current context
		else if(id == TLMP_MESSAGE_GETPASSWORD &&
			size > 1)
		{
			// The length of the USB packet is 64 bytes. 2 bytes are used for
			// the header (size + message ID) leaving 62 bytes. In theory this
			// should be null terminated, but just to be totally sure, add an
			// extra null on the end
			char pass[63];
			pass[62] = 0;
			memcpy(pass, &buffer[2], size + 1);

			if(device->callback != 0)
			((tlmpAuthCallback)device->callback)((const char*)device->user, pass);
			memset(pass, 0, 63);// as soon as we've used the password, destroy it
	    
			tlmpFree(device->user);

			device->state = TLMP_STATE_IDLE;
		}
		else
		{
			//printf("Error with 0x%X and size %d\n", id, size);
			device->state = TLMP_STATE_IDLE;
		}
		}
	}
    tlmpFree(buffer);
}

// Request libusb to receive a packet from the mooltipass
tlmpReturn tlmpReceivePacketAsync(tlmpDevice* device)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);

	{
		unsigned char* buffer = 0;
		struct libusb_transfer* transfer = 0;
		int error = 0;

		buffer = tlmpMallocArray(unsigned char, 64);
		transfer = libusb_alloc_transfer(1);

		libusb_fill_interrupt_transfer(transfer,
			device->handle,
			LIBUSB_ENDPOINT_IN | 1,
			buffer, 64,
			(libusb_transfer_cb_fn)&tlmpPacketReceived,
			device,
			10000);

		libusb_submit_transfer(transfer);

		if(error)
			tlmpReturnCode(IO);
	}
    
    tlmpReturnCode(SUCCESS);
}

// Request libusb to send a packet to the mooltipass. Will create the correct
// packet structure based on the information given
tlmpReturn tlmpSendPacketAsync(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);
	{
		struct libusb_transfer* transfer;
		int error = 0;
		unsigned char* buffer = tlmpMallocArray(unsigned char, 64);;
		size = size>62?62:size;
		buffer[0] = size;
		buffer[1] = id;
		if(size && data)
			memcpy(&buffer[2],data,size);

		transfer = libusb_alloc_transfer(1);

		libusb_fill_interrupt_transfer(transfer,
			device->handle,
			LIBUSB_ENDPOINT_OUT | 2,
			buffer, 64,
			(libusb_transfer_cb_fn)&tlmpPacketSent,
			device,
			1000);

		device->kernel = 0;
		if(libusb_kernel_driver_active(device->handle, 0))
		{
			device->kernel = 1;
			libusb_detach_kernel_driver(device->handle, 0);
		}

		device->state = id;
		error = libusb_submit_transfer(transfer);

		if(error)
			tlmpReturnCode(IO);
	}
    tlmpReturnCode(SUCCESS);
}

// hook up an external callback for when a mooltipass has been connected or disconnected
tlmpReturn tlmpSetConnectCallback(tlmpContext* context, tlmpConnectCallback callback)
{
    if(context == 0)
        tlmpReturnCode(NO_CONTEXT);

    context->connect = callback;

	{
		tlmpNode* dev = context->devices;
		while(dev)
		{
		callback(context, (tlmpDevice*)dev, 1);
		dev = dev->next;
		}
	}
    
    tlmpReturnCode(SUCCESS);
}

// TODO: Async credential request
tlmpReturn tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);
    
    if(device->state != TLMP_STATE_IDLE)
        tlmpReturnCode(BUSY);

    device->callback = callback;

	{
		unsigned char payload[62];
		unsigned int size = strlen(domain)+1;
		memcpy(payload, domain, size);
		return tlmpSendPacketAsync(device, TLMP_MESSAGE_SETCONTEXT, payload, size);
	}
}

// TODO: Async status request
tlmpReturn tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturnCode(BUSY);

    device->callback = callback;

    return tlmpSendPacketAsync(device, TLMP_MESSAGE_STATUS, 0, 0);    
}

// Find what status the mooltipass is in
tlmpReturn tlmpGetStatus(tlmpDevice* device, tlmpStatus* status)
{
    if(device == 0)
        tlmpReturnCode(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturnCode(BUSY);

    device->state = TLMP_MESSAGE_STATUS;

	{
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
    (char*)x += 0;
}
