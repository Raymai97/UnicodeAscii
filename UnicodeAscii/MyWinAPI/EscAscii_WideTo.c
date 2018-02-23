#include "EscAscii.h"

#define _ESCASCII_IMPLTYPE_DEFINED

typedef WCHAR IChar_t;
static SIZE_T const s_cbNonAscii = 6; /* \x0123 */
static BOOL const s_ForCoding_wideStrLit = TRUE;

#include "EscAscii_BaseImpl.h"

EXTERN_C
HRESULT __stdcall
EscAscii_WideToAscii(
	LPCWSTR pszWide,
	SIZE_T cchWide,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	return MyBase_ToAscii(pszWide, cchWide,
		eafFlags, ppszAscii, pcbAscii);
}
