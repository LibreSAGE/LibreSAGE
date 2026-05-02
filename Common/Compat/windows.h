#pragma once

#ifndef _WIN32
#include "windows_base.h"
#include "unknwn.h"

typedef enum eSetWindowPosFlags
{
    SWP_NOSIZE = 0x0001,
    SWP_NOMOVE = 0x0002,
    SWP_NOZORDER = 0x0004,
} eSetWindowPosFlags;

#define HWND_TOPMOST ((HWND) - 1)
#define HWND_NOTOPMOST ((HWND) - 2)

void SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
}
void GetWindowRect(HWND hWnd, RECT *pRect)
{
}

void GetClientRect(HWND hWnd, RECT *pRect);

HWND GetDesktopWindow()
{
    return (HWND)0;
}
HDC GetDC(HWND hWnd)
{
    return (HDC)0;
}
int ReleaseDC(HWND hWnd, HDC hDC)
{
    return 0;
}

void SetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
}

#define GWL_STYLE 1
DWORD GetWindowLong(HWND hWnd, int nIndex)
{
    return 0;
}

void AdjustWindowRect(RECT *pRect, DWORD dwStyle, BOOL bMenu)
{
}

#define HIWORD(value) ((((uint32_t)(value) >> 16) & 0xFFFF))
#define LOWORD(value) (((uint32_t)(value) & 0xFFFF))

#include <stdio.h>
#define MB_OK 0
#define MB_ICONEXCLAMATION 0

// MessageBox stub
inline int MessageBox(void *, const char *text, const char *caption, unsigned int type)
{
    fprintf(stderr, "%s: %s\n", caption, text);
    return 0;
}

#ifndef _MAX_FNAME
#define _MAX_FNAME 512
#endif

#ifndef _MAX_EXT
#define _MAX_EXT 16
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif

#endif