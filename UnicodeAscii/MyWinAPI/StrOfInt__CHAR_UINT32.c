#include "StrOfInt.h"

#define _STROFINT_IMPLCHAR_DEFINED
typedef CHAR	StrOfInt_ImplChar_t;

#define _STROFINT_IMPLINT_DEFINED
typedef UINT32	StrOfInt_ImplInt_t;

#include "StrOfInt_BaseImpl.h"

_STROFINT_API_(StrOfUint32, CHAR, UINT32)
{
	_STROFINT_CALL_BASEIMPL;
}

