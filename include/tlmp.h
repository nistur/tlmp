#pragma once
#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#ifdef TLMP_DYNAMIC
# ifdef WIN32
#  ifdef TLMP_BUILD
#   define TLMP_EXPORT __declspec( dllexport )
#  else
#   define TLMP_EXPORT __declspec( dllimport )
#  endif
# endif
#endif
 
#ifndef TLMP_EXPORT
# define TLMP_EXPORT
#endif

typedef int tlmpReturn;
typedef unsigned char tlmpStatus;
    
typedef struct _tlmpContext tlmpContext;
typedef struct _tlmpDevice tlmpDevice;

typedef void(*tlmpAuthCallback)(const char* login, const char* password);
typedef void(*tlmpConnectCallback)(tlmpContext* context, tlmpDevice* device);
typedef void(*tlmpStatusCallback)(tlmpContext* context, tlmpStatus status);

// tlmpReturn types    
#define TLMP_SUCCESS    0
#define TLMP_NO_CONTEXT 1
#define TLMP_NO_DEVICE  2
#define TLMP_BUSY       3

// tlmpStatus bitmasks
#define TLMP_STATUS_SMARTCARD   (1<<0)
#define TLMP_STATUS_PINSCREEN   (1<<1)
#define TLMP_STATUS_UNLOCKED    (1<<2)
#define TLMP_STATUS_UNKNOWNCARD (1<<3)
        

TLMP_EXPORT tlmpReturn   tlmpInitContext     (tlmpContext** context);
TLMP_EXPORT tlmpReturn   tlmpTerminateContext(tlmpContext** context);

TLMP_EXPORT tlmpReturn   tlmpUpdateContext   (tlmpContext* context);
TLMP_EXPORT tlmpReturn   tlmpSetConnectCallback(tlmpContext* context, tlmpConnectCallback callback);

// TODO: support async requests
    /*
TLMP_EXPORT tlmpReturn   tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback);

TLMP_EXPORT tlmpReturn   tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback);
    */
TLMP_EXPORT tlmpReturn tlmpGetStatus(tlmpDevice* device, tlmpStatus* status);

TLMP_EXPORT const char*  tlmpError();

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*__TEMPLATE_H__*/
