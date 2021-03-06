// tlmp.h - Public interface for the Tiny Little Mooltipass library
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
typedef void(*tlmpStatusCallback)(tlmpDevice* context, tlmpStatus status);

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
// note:
//  If successful, callback will be called immediately with all devices
// currently plugged in
TLMP_EXPORT tlmpReturn   tlmpSetConnectCallback (tlmpContext* context, tlmpConnectCallback callback);

// tlmpRequestAuthentication
//  Request a username and password for the provided domain.
// params:
//  device   - the device to request the credentials from
//  domain   - the domain or reference to query the credentials on (context)
//  callback - the function to call with the retrieved credentials
// returns:
//  TLMP_SUCCESS   - Request successfully sent
//  TLMP_NO_DEVICE - Null or invalid device provided
//  TLMP_BUSY      - Device is currently processing another request
//  TLMP_IO        - IO error while communicating with device
// note:
//  If the mooltipass doesn't return a valid credential pair, either if the
// user doesn't allow access, or if the mooltipass doesn't have a credential pair
// for the provided domain, the callback will _not_ be called.
TLMP_EXPORT tlmpReturn   tlmpRequestAuthentication(tlmpDevice* device, const char* domain, tlmpAuthCallback callback);

// tlmpRequestStatus
//  Request the status of the mooltipass device
// params:
//  device   - the device to request the status of
//  callback - the function to call with the retrieved status
// returns:
//  TLMP_SUCCESS   - Request successfully sent
//  TLMP_NO_DEVICE - Null or invalid device provided
//  TLMP_BUSY      - Device is currently processing another request
//  TLMP_IO        - IO error while communicating with device
TLMP_EXPORT tlmpReturn   tlmpRequestStatus(tlmpDevice* device, tlmpStatusCallback callback);


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
