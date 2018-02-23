#ifndef _MYWINAPI_STROFINT_H
#define _MYWINAPI_STROFINT_H

#include <Windows.h>
#include "StrOfInt_BaseTsd.h"

/*
	StrOfInt is designed to be low-level and multi-purpose.
	It doesn't not zero memory.
	It doesn't not do any formatting.
	It doesn't prepend any prefix like "0x".
	The only thing it prepends is the negative sign
	when 'radix' is 10 and 'val' is negative value.

	'radix' must between 2 and 16.
	'psz' must be specified (non-NULL).
	'nMax' specify max number of chars to be written.
	If 'nMax' is -1, it uses strlen(psz).

	If succeed, return pointer to the first written char.
	Otherwise return NULL.
	
	To keep API lean, only distinct types are defined.
	For other type, see this:
	BYTE		| StrOfUint8((UINT8)value, ...)
	WORD		| StrOfUint16((UINT16)value, ...)
	DWORD		| StrOfUint32((UINT32)value, ...)
	QWORD		| StrOfUint64((UINT64)value, ...)
	LONG		| StrOfInt32((INT32)value, ...)
	ULONG		| StrOfUint32((UINT32)value, ...)
	LONGLONG	| StrOfInt64((INT64)value, ...)
	ULONGLONG	| StrOfUint64((UINT64)value, ...)
	WPARAM		| StrOfUintPtr((UINT_PTR)value, ...)
	LPARAM		| StrOfIntPtr((INT_PTR)value, ...)
	LRESULT		| StrOfIntPtr((INT_PTR)value, ...)
	SIZE_T		| StrOtUintPtr((UINT_PTR)value, ...)
	void*		| StrOfUintPtr((UINT_PTR)value, ...)
*/

#define _STROFINT_API_(api_name, char_t, int_t) \
	EXTERN_C char_t * __stdcall api_name( \
		int_t val, int radix, char_t *psz, int nMax)


_STROFINT_API_(StrOfInt8, CHAR, INT8);
_STROFINT_API_(WcsOfInt8, WCHAR, INT8);
_STROFINT_API_(TcsOfInt8, TCHAR, INT8);

_STROFINT_API_(StrOfUint8, CHAR, UINT8);
_STROFINT_API_(WcsOfUint8, WCHAR, UINT8);
_STROFINT_API_(TcsOfUint8, TCHAR, UINT8);

_STROFINT_API_(StrOfInt16, CHAR, INT16);
_STROFINT_API_(WcsOfInt16, WCHAR, INT16);
_STROFINT_API_(TcsOfInt16, TCHAR, INT16);

_STROFINT_API_(StrOfUint16, CHAR, UINT16);
_STROFINT_API_(WcsOfUint16, WCHAR, UINT16);
_STROFINT_API_(TcsOfUint16, TCHAR, UINT16);

_STROFINT_API_(StrOfInt32, CHAR, INT32);
_STROFINT_API_(WcsOfInt32, WCHAR, INT32);
_STROFINT_API_(TcsOfInt32, TCHAR, INT32);

_STROFINT_API_(StrOfUint32, CHAR, UINT32);
_STROFINT_API_(WcsOfUint32, WCHAR, UINT32);
_STROFINT_API_(TcsOfUint32, TCHAR, UINT32);

_STROFINT_API_(StrOfInt64, CHAR, INT64);
_STROFINT_API_(WcsOfInt64, WCHAR, INT64);
_STROFINT_API_(TcsOfInt64, TCHAR, INT64);

_STROFINT_API_(StrOfUint64, CHAR, UINT64);
_STROFINT_API_(WcsOfUint64, WCHAR, UINT64);
_STROFINT_API_(TcsOfUint64, TCHAR, UINT64);

_STROFINT_API_(StrOfIntPtr, CHAR, INT_PTR);
_STROFINT_API_(WcsOfIntPtr, WCHAR, INT_PTR);
_STROFINT_API_(TcsOfIntPtr, TCHAR, INT_PTR);

_STROFINT_API_(StrOfUintPtr, CHAR, UINT_PTR);
_STROFINT_API_(WcsOfUintPtr, WCHAR, UINT_PTR);
_STROFINT_API_(TcsOfUintPtr, TCHAR, UINT_PTR);


#endif/*_MYWINAPI_STROFINT_H*/
