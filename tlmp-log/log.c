// log.c - simple mooltipass test program
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

#include "stdio.h"
#include "tlmp_internal.h"
#include "libusb.h"
#include "pthread.h"
#include "unistd.h"

int running = 0;
tlmpContext* ctx;
tlmpDevice* dev;

void _unused(void* x){ x += 0; }
#define UNUSED(x) _unused((void*)&x)

#define REFRESH (1000000 / 60)

void* update(void* user)
{
    UNUSED(user);
    while( running )
    {
    	tlmpUpdateContext(ctx);
    	usleep(REFRESH);
    }
    return 0;
}

void deviceConnected(tlmpContext* context, tlmpDevice* device, int connected)
{
    UNUSED(context);
    if(connected)
        dev = device;
    else if(dev == device)
        dev = NULL;
}

int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    printf("Start\n");
    int c;
    pthread_t threadID;
    tlmpInitContext(&ctx);

    tlmpSetConnectCallback(ctx, deviceConnected);

    running = 1;
    
    pthread_create(&threadID, NULL, update, NULL);
    while(1)
    {
	scanf("%d", &c);
	if(c==0)
	    break;
        else if(dev)
	{
	    tlmpStatus stat = 0;
	    tlmpGetStatus(dev, &stat);
	    if(stat & TLMP_STATUS_SMARTCARD)
		printf("Smartcard inserted\n");
	    if(stat & TLMP_STATUS_PINSCREEN)
		printf("On unlock screen\n");
	    if(stat & TLMP_STATUS_UNLOCKED)
		printf("Device unlocked\n");
	    if(stat & TLMP_STATUS_UNKNOWNCARD)
		printf("Unknown card inserted\n");
	}
    }
    running = 0;
    tlmpTerminateContext(&ctx);
    pthread_join(threadID, NULL);

    printf("End\n");
}
