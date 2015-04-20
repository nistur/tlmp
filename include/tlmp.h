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

typedef struct _tlmpContext tlmpContext;
typedef struct _tlmpDevice tlmpDevice;

#define TLMP_SUCCESS    0
#define TLMP_NO_CONTEXT 1

TLMP_EXPORT tlmpReturn   tlmpInitContext     (tlmpContext** context);
TLMP_EXPORT tlmpReturn   tlmpTerminateContext(tlmpContext** context);
TLMP_EXPORT tlmpReturn   tlmpUpdateContext(tlmpContext* context);


TLMP_EXPORT const char*  tlmpError();

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*__TEMPLATE_H__*/
