#include <Windows.h>

/*
	------ Force inline memset ------
	To avoid MSVCRT dependency.
*/

#pragma intrinsic(memset)

EXTERN_C void * __cdecl memset(void *, int, size_t);

#pragma function(memset)

EXTERN_C void * __cdecl memset(void *pBuf, int val, size_t cb)
{
	unsigned char *p = pBuf;
	while (cb-- > 0) {
		*p++ = (unsigned char)(val);
	}
	return pBuf;
}


#pragma intrinsic(memcpy)

EXTERN_C void * __cdecl memcpy(void *, void const *, size_t);

#pragma function(memcpy)

EXTERN_C void * __cdecl memcpy(void *pDst, void const *pSrc, size_t cbSrc)
{
	typedef unsigned char byte_t;
	byte_t *p = pDst;
	byte_t const *q = pSrc;
	while (cbSrc-- > 0) {
		*p++ = *q++;
	}
	return pDst;
}
