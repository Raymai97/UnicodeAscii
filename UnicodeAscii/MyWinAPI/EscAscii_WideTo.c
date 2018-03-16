#include "EscAscii.h"

#define _ESCASCII_IMPLTYPE_DEFINED

typedef WCHAR IChar_t;

#define _ESCASCII_DquoteOn		*p++ ='L'; *p++ = '\"'
#define _ESCASCII_DquoteOff		*p++ = '\"'
#define _ESCASCII_DquotePairCB	(3)
#define _ESCASCII_NonAsciiCB	(6) /* \x0123 */

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
	return MyBase_EscapeToAscii(pszWide, cchWide,
		eafFlags, ppszAscii, pcbAscii);
}
