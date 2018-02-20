#include "EscAscii.h"

#ifndef MYWINAPI_PREFER_USER_MEM_ALLOCATOR

#include "EscAscii_BaseImpl.h"

EXTERN_C
void __stdcall
EscAscii_Free(
	LPVOID ptr)
{
	MyFree(ptr);
}

#endif/*MYWINAPI_PREFER_USER_MEM_ALLOCATOR*/

