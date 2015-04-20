#include "stdio.h"
#include "tlmp_internal.h"
#include "libusb-1.0/libusb.h"
#include "pthread.h"
#include "unistd.h"

int running = 0;
tlmpContext* ctx;

#define REFRESH (1000000 / 60)

void* update(void* user)
{
    while( running )
    {
	tlmpUpdateContext(ctx);
	usleep(REFRESH);
    }
    return 0;
}

int main(int argc, char** argv)
{
    printf("Start\n");
    int c;
    pthread_t threadID;
    tlmpInitContext(&ctx);

    running = 1;
    
    pthread_create(&threadID, NULL, update, NULL);
    while(1)
    {
	scanf("%d", &c);
	if(c==0)
	    break;
    }
    running = 0;
    tlmpTerminateContext(&ctx);
    pthread_join(threadID, NULL);

    printf("End\n");
}
