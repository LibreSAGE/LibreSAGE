#pragma once
#include <stddef.h>
#include <stdint.h>

// Prevent bool.h from setting up its own bool type if it gets included after this file. This is needed for some of the older compression code that still uses bool.h, but we want to be able to use our own Bool type in the rest of the codebase.
#define TRUE_FALSE_DEFINED

#ifdef __GNUC__
#if 0 //__has_attribute(cdecl)
#define __cdecl __attribute__((cdecl))
#else
#define _cdecl
#define __cdecl
#endif
#endif

// OutputDebugString
#ifndef OutputDebugString
#define OutputDebugString(str) printf("%s\n", str)
#endif

#define INVALID_HANDLE_VALUE 0

#ifdef _WIN32
#define HANDLE void *
#endif

int GetLastError()
{
    return 0;
}
#define TRUE 1
#define FALSE 0

#include "string_compat.h"
#include "time_compat.h"
#include "wchar_compat.h"
#include "intrin_compat.h"

#ifndef _WIN32
#include <stdio.h>
#define MB_OK 0
#define MB_ICONEXCLAMATION 0

// MessageBox stub
inline int MessageBox(void *, const char *text, const char *caption, unsigned int type)
{
    fprintf(stderr, "%s: %s\n", caption, text);
    return 0;
}
#endif