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
#ifndef __TLMP_MESSAGE_IDS_H__
#define __TLMP_MESSAGE_IDS_H__

#define TLMP_MESSAGE_DEBUG            0xA0 
#define TLMP_MESSAGE_PING             0xA1
#define TLMP_MESSAGE_VERSION          0xA2
#define TLMP_MESSAGE_SETCONTEXT       0xA3
#define TLMP_MESSAGE_GETLOGIN         0xA4
#define TLMP_MESSAGE_GETPASSWORD      0xA5
#define TLMP_MESSAGE_SETLOGIN         0xA6
#define TLMP_MESSAGE_SETPASSWORD      0xA7
#define TLMP_MESSAGE_CHECKPASSWORD    0xA8
#define TLMP_MESSAGE_ADDCONTEXT       0xA9
#define TLMP_MESSAGE_GETRANDOM        0xAC
#define TLMP_MESSAGE_MEMMANAGEMENT    0xAD
#define TLMP_MESSAGE_MEDIABEGIN       0xAF 
#define TLMP_MESSAGE_MEDIAEND         0xB0
#define TLMP_MESSAGE_SETPARAMETER     0xB1
#define TLMP_MESSAGE_GETPARAMETER     0xB2
#define TLMP_MESSAGE_RESETCARD        0xB3
#define TLMP_MESSAGE_READCARDLOGIN    0xB4
#define TLMP_MESSAGE_READCARDPASSWORD 0xB5
#define TLMP_MESSAGE_SETCARDLOGIN     0xB6
#define TLMP_MESSAGE_SETCARDPASSWORD  0xB7
#define TLMP_MESSAGE_ADDUNKNOWNCARD   0xB8
#define TLMP_MESSAGE_STATUS           0xB9
#define TLMP_MESSAGE_SETDATE          0xBB
#define TLMP_MESSAGE_SETUID           0xBC
#define TLMP_MESSAGE_GETUID           0xBD
#define TLMP_MESSAGE_SETDATACONTEXT   0xBE
#define TLMP_MESSAGE_ADDDATACONTEXT   0xBF
#define TLMP_MESSAGE_WRITEDATACONTEXT 0xC0
#define TLMP_MESSAGE_READDATACONTEXT  0xC1
#define TLMP_MESSAGE_GETCARDCPY       0xC2

#endif/*__TLMP_MESSAGE_IDS_H__*/
