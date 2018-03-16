#include "EscAscii.h"

#define _ESCASCII_IMPLTYPE_DEFINED

typedef CHAR IChar_t;

#define _ESCASCII_DquoteOn		*p++ = '\"'
#define _ESCASCII_DquoteOff		*p++ = '\"'
#define _ESCASCII_DquotePairCB	(2)
#define _ESCASCII_NonAsciiCB	(4) /* \x23 */

#include "EscAscii_BaseImpl.h"

EXTERN_C
HRESULT __stdcall
EscAscii_MultiToAscii(
	LPCSTR pszMulti,
	SIZE_T cbMulti,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	return MyBase_EscapeToAscii(pszMulti, cbMulti,
		eafFlags, ppszAscii, pcbAscii);
}
