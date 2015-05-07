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


    if(context->connect != 0)
        context->connect(context, device);
    
    tlmpReturn(SUCCESS);
}

void tlmpPacketSent(struct libusb_transfer* transfer)
{
}

tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    unsigned char buffer[64];
    size = size>62?62:size;
    buffer[0] = size;
    buffer[1] = id;
    memcpy(&buffer[2],data,size);
    
    
    struct libusb_transfer* transfer = libusb_alloc_transfer(1);
    libusb_fill_control_transfer(transfer,
                                 device->dev,
                                 data,
                                 tlmpPacketSent,
                                 device,
                                 1000);
                                 
    
    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpSetConnectCallback(tlmpContext* context, tlmpConnectCallback callback)
{
    if(context == 0)
        tlmpReturn(NO_CONTEXT);

    context->connect = callback;
    
    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);
    
    if(device->callback)
        tlmpReturn(BUSY);
    
    device->callback = callback;
    device->state = TLMP_STATE_WAITFORLOGIN;
    
    tlmpReturn(SUCCESS);
}

const char* tlmpError()
{
    return g_tlmpErrors[g_tlmpError];
}
