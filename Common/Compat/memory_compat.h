#pragma once
#include <stdlib.h>

#define GMEM_ZEROINIT 0x0040
#define GMEM_FIXED 0x0010

void* GlobalAlloc(unsigned int flags, unsigned int numBytes)
{
    return malloc(numBytes);
}

void GlobalFree(void* ptr)
{
    free(ptr);
}

#ifndef _WIN32
#include <windows_base.h>

void GlobalMemoryStatus(MEMORYSTATUS* memStatus)
{
    memStatus->dwLength = sizeof(MEMORYSTATUS);
    memStatus->dwTotalPhys = 0;
}

#define HEAP_ZERO_MEMORY 0x00000008
void* HeapAlloc(void* heap, unsigned int flags, unsigned int numBytes)
{
    return malloc(numBytes);
}

void HeapFree(void* heap, unsigned int flags, void* ptr)
{
    free(ptr);
}

void* GetProcessHeap()
{
    return nullptr;
}
#endif