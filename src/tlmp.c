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

    int error = libusb_open(device->dev, &device->handle); 
    if( error )
        printf( "open failed with error %d\n", error );
    
    printf( "handle: 0x%X\n", device->handle );


    if(context->connect != 0)
        context->connect(context, device);
    
    tlmpReturn(SUCCESS);
}

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

tlmpReturn tlmpSendPacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    unsigned char buffer[64];
    size = size>62?62:size;
    buffer[0] = size;
    buffer[1] = id;
    memcpy(&buffer[2],data,size);
    size += 2;

    unsigned char hasKernel = 0;
    if(libusb_kernel_driver_active(device->handle, 0))
    {
	hasKernel = 1;
	libusb_detach_kernel_driver(device->handle, 0);
    }
    
    int error = libusb_claim_interface(device->handle, 0);
    if(error)
	printf("couldn't claim interface (%d) %s\n", error, libusb_strerror(error));
    
    
    int actual_size = 0;
    error = libusb_interrupt_transfer(device->handle,
				      LIBUSB_ENDPOINT_OUT | 2,
				      buffer, size, &actual_size, 0 );

    if( error )
        printf( "transfer failed with error %d\n", error );
    if(size != actual_size)
        printf("didn't send enough data %d/%d\n", actual_size, size );

    libusb_release_interface(device->handle, 0);


    if(hasKernel)
	libusb_attach_kernel_driver(device->handle, 0);
    
    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpReceivePacket(tlmpDevice* device, unsigned char id, unsigned char* data, unsigned char size)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    unsigned char buffer[64];
    unsigned char hasKernel = 0;
    if(libusb_kernel_driver_active(device->handle, 0))
    {
	hasKernel = 1;
	libusb_detach_kernel_driver(device->handle, 0);
    }
    
    int error = libusb_claim_interface(device->handle, 0);
    if(error)
	printf("couldn't claim interface (%d) %s\n", error, libusb_strerror(error));
    
    
    int actual_size = 0;
    error = libusb_interrupt_transfer(device->handle,
				      LIBUSB_ENDPOINT_IN | 1,
				      buffer, 64, &actual_size, 0 );

    if( error )
        printf( "transfer failed with error %d\n", error );
    if(64 != actual_size)
        printf("didn't receive enough data %d/%d\n", actual_size, size );

    libusb_release_interface(device->handle, 0);


    if(hasKernel)
	libusb_attach_kernel_driver(device->handle, 0);

    if(buffer[1] != id)
	printf("Received invalid message\n");
    if(size<buffer[0])
	printf("Received more data than anticipated %d/%d\n", size, buffer[0]);
    memcpy(data, &buffer[2], size);
    
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
    
    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);
    
    device->callback = (void*)callback;
    device->state = TLMP_STATE_WAITFORLOGIN;
    
    tlmpReturn(SUCCESS);
}

tlmpReturn tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);

    device->state = TLMP_STATE_WAITFORSTATUS;
    device->callback = (void*)callback;

    tlmpStatus stat = 0;
    tlmpReturn err =  tlmpSendPacket(device,
				     TLMP_MESSAGE_STATUS,
				     &stat, 1);
    err = tlmpReceivePacket(device,
			    TLMP_MESSAGE_STATUS,
			    &stat, 1);
    return err;
    
}

tlmpReturn tlmpGetStatus(tlmpDevice* device, tlmpStatus* status)
{
    if(device == 0)
        tlmpReturn(NO_DEVICE);

    if(device->state != TLMP_STATE_IDLE)
        tlmpReturn(BUSY);

    device->state = TLMP_STATE_WAITFORSTATUS;

    tlmpStatus stat = 0;
    tlmpReturn err =  tlmpSendPacket(device,
				     TLMP_MESSAGE_STATUS,
				     &stat, sizeof(stat));
    err = tlmpReceivePacket(device,
			    TLMP_MESSAGE_STATUS,
			    status, sizeof(tlmpStatus));
    device->state = TLMP_STATE_IDLE;
    return err;
    
}

void* tlmpMallocInternal(int n)
{
    void* p = malloc(n);
    memset(p,0,n);
    return p;
}

void tlmpFreeInternal(void** p)
{
    if(p && *p)
    {
        free(*p);
        *p = 0;
    }      
}

const char* tlmpError()
{
    return g_tlmpErrors[g_tlmpError];
}
