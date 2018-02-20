#ifndef MYWINAPI_ESCASCII_H
#define MYWINAPI_ESCASCII_H

#include <Windows.h>

#ifndef MYWINAPI_PREFER_USER_MEM_ALLOCATOR

EXTERN_C
void __stdcall
EscAscii_Free(
	LPVOID ptr
);

#else /*MYWINAPI_PREFER_USER_MEM_ALLOCATOR*/

#ifndef MYWINAPI_IDENTIFIER_MEMALLOCZERO
#define MYWINAPI_IDENTIFIER_MEMALLOCZERO	MemAllocZero
#endif

#ifndef MYWINAPI_IDENTIFIER_MEMFREE
#define MYWINAPI_IDENTIFIER_MEMFREE			MemFree
#endif

EXTERN_C LPVOID MYWINAPI_IDENTIFIER_MEMALLOCZERO(SIZE_T cb);
EXTERN_C void MYWINAPI_IDENTIFIER_MEMFREE(LPVOID ptr);

#endif/*MYWINAPI_PREFER_USER_MEM_ALLOCATOR*/


enum EscAsciiFlag {
	/*
		The continuous byte of encoding may look like ASCII (< 0x80)
		so if curr char >= 0x80, never consider next char as ASCII.
		Useful for DBCS like Shift-JIS.
	*/
	EAF_DbcsConti = 1 << 0,

	EAF__max = 1 << 30
};


/*
	If cchWide / cbMulti is zero,
	..process until null-termi in pszWide / pszMulti.

	ppszAscii is optional if pcbAscii is not NULL.
	pcbAscii is optional if ppszAscii is not NULL.

	If pcbAscii not NULL,
	..*pcbAscii = number of chars written without null-termi.
*/

EXTERN_C
HRESULT __stdcall
EscAscii_FromWide(
	LPCWSTR pszWide,
	SIZE_T cchWide,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii
);

EXTERN_C
HRESULT __stdcall
EscAscii_FromMulti(
	LPCSTR pszMulti,
	SIZE_T cbMulti,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii
);

#endif/*MYWINAPI_ESCASCII_H*/
