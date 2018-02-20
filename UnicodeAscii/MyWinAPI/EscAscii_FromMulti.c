#include "EscAscii.h"

#define _ESCASCII_IMPLTYPE_DEFINED

typedef CHAR IChar_t;
static SIZE_T const s_cbNonAscii = 4;

#include "EscAscii_BaseImpl.h"

EXTERN_C
HRESULT __stdcall
EscAscii_FromMulti(
	LPCSTR pszMulti,
	SIZE_T cbMulti,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	return MyBase_ToAscii(pszMulti, cbMulti,
		eafFlags, ppszAscii, pcbAscii);
}
