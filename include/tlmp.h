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
typedef void(*tlmpConnectCallback)(tlmpContext* context, tlmpDevice* device, int connected);
typedef void(*tlmpStatusCallback)(tlmpContext* context, tlmpStatus status);

// tlmpReturn types    
#define TLMP_SUCCESS    0
#define TLMP_NO_CONTEXT 1
#define TLMP_NO_DEVICE  2
#define TLMP_BUSY       3
#define TLMP_PERMISSION 4
#define TLMP_IO         5
#define TLMP_STORAGE    6

// tlmpStatus bitmasks
#define TLMP_STATUS_SMARTCARD   (1<<0) // user has inserted smartcard into device
#define TLMP_STATUS_PINSCREEN   (1<<1) // device is displaying the unlock screen
#define TLMP_STATUS_UNLOCKED    (1<<2) // device is unlocked
#define TLMP_STATUS_UNKNOWNCARD (1<<3) // user has inserted an unknown card into device
        

// tlmpInitContext
//  Instantiate a new tlmp context
// params:
//  context - The mooltipass library context to initialise
// returns:
//  TLMP_SUCCESS    - context successfully initialised
//  TLMP_NO_CONTEXT - Null context pointer provided
//  TLMP_STORAGE    - Invalid memory available to initialise context
TLMP_EXPORT tlmpReturn   tlmpInitContext        (tlmpContext** context);

// tlmpTerminateContext
//  Cleans up the tlmp context
// params:
//  context - The mooltipass library context to clean up
// returns:
//  TLMP_SUCCESS    - context successfully cleaned up
//  TLMP_NO_CONTEXT - Null context pointer provided
TLMP_EXPORT tlmpReturn   tlmpTerminateContext   (tlmpContext** context);

// tlmpUpdateContext
//  Updates the library context
// params:
//  context - The context to update
// returns:
//  TLMP_SUCCESS    - The context updated successfully
//  TLMP_NO_CONTEXT - Null context pointer provided
// note:
//  This needs to be called repeatedly to process USB messages
TLMP_EXPORT tlmpReturn   tlmpUpdateContext      (tlmpContext* context);

// tlmpSetConnectCallback
//  Sets a callback to be called when a mooltipass device has been connected or disconnected
// params:
//  context  - The context to respond to the callback
//  callback - The function to be called when a device is connected
// returns:
//  TLMP_SUCCESS    - Setting the callback function has been successful
//  TLMP_NO_CONTEXT - Null context pointer provided
TLMP_EXPORT tlmpReturn   tlmpSetConnectCallback (tlmpContext* context, tlmpConnectCallback callback);

// TODO: support async requests
/*
TLMP_EXPORT tlmpReturn   tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback);

TLMP_EXPORT tlmpReturn   tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback);
*/

// tlmpGetStatus
//  Gets the current status of a specific mooltipass
// params:
//  device - The mooltipass device to query the status of
//  status - A pointer to a status type
// returns:
//  TLMP_SUCCESS    - status correctly retrieved from the mooltipass
//  TLMP_NO_DEVICE  - invalid device passed
//  TLMP_PERMISSION - user doesn't have permissions to access USB
//  TLMP_IO         - I/O error in communications
TLMP_EXPORT tlmpReturn tlmpGetStatus            (tlmpDevice* device, tlmpStatus* status);

// tlmpError
//  Returns end-user ready error message
// returns:
//  Null terminated string with human readable error message
TLMP_EXPORT const char*  tlmpError();

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*__TEMPLATE_H__*/
