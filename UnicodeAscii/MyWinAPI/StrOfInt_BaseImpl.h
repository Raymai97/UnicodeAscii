#include <Windows.h>

#ifndef _STROFINT_IMPLCHAR_DEFINED
typedef CHAR	StrOfInt_ImplChar_t;
#endif/*_STROFINT_IMPLCHAR_DEFINED*/

#ifndef _STROFINT_IMPLINT_DEFINED
typedef INT64	StrOfInt_ImplInt_t;
#endif/*_STROFINT_IMPLINT_DEFINED*/


typedef StrOfInt_ImplChar_t		IChar_t;
typedef StrOfInt_ImplInt_t		IInt_t;

#define _STROFINT_CALL_BASEIMPL \
	return My_BaseImpl(val, radix, psz, nMax)

static IChar_t * My_BaseImpl(
	IInt_t val, int radix, IChar_t *psz, int nMax)
{
	BOOL const isNeg = (val < 0);
	int iStr = 0, iChr = 0;
	
	if (nMax == -1) {
		IChar_t *p = psz;
		for (; *p; ++p);
		nMax = (int)(p - psz);
	}
	if (radix < 2 || radix > 16) { return NULL; }
	if (psz == NULL || nMax < 1) { return NULL; }

	for (iStr = nMax; iStr-- > 0;) {
		iChr = val % radix;
		if (iChr < 0) { iChr *= -1; }
		psz[iStr] = "0123456789ABCDEF"[iChr];
		val /= (IInt_t)radix;
		if (!val) { break; }
	}
	/* Return NULL if nMax too small */
	if (val != 0) { return NULL; }
	if (radix == 10 && isNeg) {
		if (--iStr < 0) { return NULL; }
		psz[iStr] = '-';
	}
	return &psz[iStr];
}
