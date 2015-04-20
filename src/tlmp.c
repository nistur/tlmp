#include "tlmp_internal.h"
#include "sys/time.h"

tlmpReturn tlmpClearContext(tlmpContext* context)
{
    tlmpNode* dev = context->devices;
    while(dev)
    {
	tlmpNode* next = dev->next;
	tlmpFree(dev);
	dev = next;
    }
    context->devices = 0;

    tlmpReturn(SUCCESS);
}

int tlmpHotplugCallback(libusb_context* lib, libusb_device* dev, libusb_hotplug_event evt, void* context)
{
    tlmpDevice* device = tlmpMalloc(tlmpDevice);
    device->dev = dev;

    tlmpAddDevice((tlmpContext*)context, device);
    
    printf("Ping\n");
}

tlmpReturn tlmpInitContext(tlmpContext** context)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);
    *context = tlmpMalloc(tlmpContext);
    if(tlmpClearContext(*context) != TLMP_SUCCESS)
	   tlmpTerminateContext(context);

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

tlmpReturn tlmpUpdateContext(tlmpContext* context)
{
    if(context == 0)
	tlmpReturn(NO_CONTEXT);

    struct timeval tv;
    memset(&tv, 0, sizeof(tv));

    libusb_handle_events_timeout(context->lib, &tv);
    
    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpAddDevice(tlmpContext* context, tlmpDevice* device)
{
    if(context == 0)
	tlmpReturn(NO_CONTEXT);

    tlmpNode** tail = &context->devices;
    while(*tail)
	tail = &(*tail)->next;
    *tail = &device->header;
    
    tlmpReturn(SUCCESS);
}

const char* tlmpError()
{
    return g_tlmpErrors[g_tlmpError];
}
