#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_PLAY_CHN 9

#include <iostream>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <math.h>

using namespace std;

#include "Classes/VorxTimer.h"
#include "Classes/MutexInteger.h"
#include "Classes/VorxThread.h"
using namespace vfc;

#define MAX_READ_PACKETS                    128
#define MP4_ERROR_SUCCESS                   0
// free the p and set to NULL.
// p must be a T*.
#define safe_freep(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0
// please use the freepa(T[]) to free an array,
// or the behavior is undefined.
#define safe_freepa(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#if 0
#define UINT64 unsigned long long
#define INT64 long long
#define BYTE unsigned char
#define ULONG unsigned long
#define UINT unsigned int
#define WORD unsigned short
#define DWORD unsigned long
#define LPCTSTR const char*
#define BOOL int
#define TRUE 1
#define FALSE 0
#define LPVOID void*
#define HANDLE void*
#define UINT32 unsigned long
#define INT32 long
#endif

#endif
