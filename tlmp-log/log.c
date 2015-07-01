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
#ifndef WIN32
#include "pthread.h"
#include "unistd.h"
#else
#include <windows.h>
#endif

int running = 0;
tlmpContext* ctx;
tlmpDevice* dev;

void _unused(void* x){ (char*)x += 0; }
#define UNUSED(x) _unused((void*)&x)

#define REFRESH (1000000 / 60)

#ifdef WIN32
void usleep(__int64 usec)
{
	HANDLE timer; 
	LARGE_INTEGER ft; 

	ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL); 
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
	WaitForSingleObject(timer, INFINITE); 
	CloseHandle(timer); 
}
#endif

#ifndef WIN32
void* update(void* user)
#else
DWORD WINAPI update(LPVOID user)
#endif
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
    printf("got device\n");
    if(connected)
        dev = device;
    else if(dev == device)
        dev = NULL;
}

void onRequest(tlmpDevice* context, tlmpStatus status)
{
    printf("Hello 0x%X\n", status);
}

void onAuth(const char* login, const char* pass)
{
    printf("Got auth %s:%s\n", login, pass);
}
    
int main(int argc, char** argv)
{
	int c;
#ifndef WIN32
    pthread_t threadID;
#else
	DWORD threadID;
	HANDLE threadHandle;
#endif
	UNUSED(argc);
	UNUSED(argv);
	printf("Start\n");
    tlmpInitContext(&ctx);

    tlmpSetConnectCallback(ctx, deviceConnected);

    running = 1;
    
#ifndef WIN32
    pthread_create(&threadID, NULL, update, NULL);
#else
	threadHandle = CreateThread(NULL, 0, update, NULL, 0, &threadID);
#endif
    while(1)
    {
	scanf_s("%d", &c);
	if(c==0)
	    break;
        else if(dev)
		{
			char host[62];
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
			//gethostname(host, 62);
			sprintf( host, "faun" );
			tlmpRequestAuthentication(dev, host, onAuth);
	}
    }
    running = 0;
#ifndef WIN32
    pthread_join(threadID, NULL);
#else
	WaitForSingleObject(threadHandle, INFINITE);
	CloseHandle(threadHandle);
#endif
    tlmpTerminateContext(&ctx);

    printf("End\n");
}
