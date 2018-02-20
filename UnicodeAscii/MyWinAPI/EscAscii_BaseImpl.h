#include "EscAscii.h"

#ifndef MYWINAPI_PREFER_USER_MEM_ALLOCATOR

static LPVOID MyAllocZero(SIZE_T cb)
{
	LPVOID ptr = HeapAlloc(GetProcessHeap(), 0, cb);
	if (ptr) { ZeroMemory(ptr, cb); }
	return ptr;
}

static void MyFree(LPVOID ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

#else /*MYWINAPI_PREFER_USER_MEM_ALLOCATOR*/

static LPVOID MyAllocZero(SIZE_T cb)
{
	return MYWINAPI_IDENTIFIER_MEMALLOCZERO(cb);
}

static void MyFree(LPVOID ptr)
{
	MYWINAPI_IDENTIFIER_MEMFREE(ptr);
}

#endif/*MYWINAPI_PREFER_USER_MEM_ALLOCATOR*/


#ifndef _ESCASCII_IMPLTYPE_DEFINED
#define _ESCASCII_IMPLTYPE_DEFINED

typedef CHAR IChar_t;
static SIZE_T const s_cbNonAscii = 4;

#endif


#define MY_HasFlag(flags, f) \
	( ((flags) & (f)) == (f) )


static SIZE_T MyStrLen(IChar_t const *psz)
{
	IChar_t const *p = psz;
	for (; *p; ++p);
	return (SIZE_T)(p - psz);
}

static void MyAppendHex(CHAR **pp, IChar_t c)
{
	static CHAR const hexChars[] = "0123456789ABCDEF";
	CHAR *p = *pp;
	SIZE_T i = sizeof(c);
	while (i --> 0)
	{
		*p++ = hexChars[(c >> (i * 8 + 4)) & 0xF];
		*p++ = hexChars[(c >> (i * 8 + 0)) & 0xF];
	}
	*pp = p;
}


static HRESULT MyBase_ToAscii(
	IChar_t const *pszSrc,
	SIZE_T nSrc,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	BOOL const dbcsConti = MY_HasFlag(eafFlags, EAF_DbcsConti);
	HRESULT hr = 0;
	BOOL isDbcsConti = FALSE;
	SIZE_T i = 0, cbAscii = 0;
	LPSTR pszAscii = NULL;
	CHAR *p = NULL;

	if (!pszSrc || (!ppszAscii && !pcbAscii)) {
		hr = E_INVALIDARG; goto eof;
	}

	if (nSrc == 0) {
		nSrc = MyStrLen(pszSrc);
	}
	for (i = 0; i < nSrc; ++i)
	{
		IChar_t const c = pszSrc[i];
		if (isDbcsConti) {
			isDbcsConti = FALSE;
		}
		else if ((unsigned)c < 0x80) {
			cbAscii += sizeof(CHAR);
			continue;
		}
		else if (dbcsConti) {
			isDbcsConti = TRUE;
		}
		cbAscii += s_cbNonAscii;
	}
	if (!ppszAscii) goto eof;

	pszAscii = MyAllocZero(cbAscii + 1);
	if (!pszAscii) {
		hr = E_OUTOFMEMORY; goto eof;
	}
	isDbcsConti = FALSE;
	p = pszAscii;
	for (i = 0; i < nSrc; ++i)
	{
		IChar_t const c = pszSrc[i];
		if (isDbcsConti) {
			isDbcsConti = FALSE;
		}
		else if ((unsigned)c < 0x80) {
			*p++ = (c & 0xFF);
			continue;
		}
		else if (dbcsConti) {
			isDbcsConti = TRUE;
		}
		*p++ = '\\';
		*p++ = 'x';
		MyAppendHex(&p, c);
	}
	*ppszAscii = pszAscii;
eof:
	if (pcbAscii) { *pcbAscii = cbAscii; }
	return hr;
}
