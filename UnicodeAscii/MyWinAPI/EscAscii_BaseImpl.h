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

#define _ESCASCII_DquoteOn		*p++ = '\"'
#define _ESCASCII_DquoteOff		*p++ = '\"'
#define _ESCASCII_DquotePairCB	(2)
#define _ESCASCII_NonAsciiCB	(4) /* \x23 */

#endif/*_ESCASCII_IMPLTYPE_DEFINED*/


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
	CHAR const *pszHex = "0123456789ABCDEF";
	CHAR *p = *pp;
	SIZE_T i = sizeof(c);
	while (i --> 0)
	{
		*p++ = pszHex[(c >> (i * 8 + 4)) & 0xF];
		*p++ = pszHex[(c >> (i * 8 + 0)) & 0xF];
	}
	*pp = p;
}


/* ForCoding declare ............................. */

#define FC_DQUOTE_OFF		_ESCASCII_DquoteOff
#define FC_DQUOTE_ON		_ESCASCII_DquoteOn
#define FC_DQUOTE_PAIR_CB	_ESCASCII_DquotePairCB

#define FC_LINEBREAKCODE \
	if (MY_HasFlag(eafFlags, EAF_FC_CodeCrLf)) \
	{ *p++ = '\\'; *p++ = 'r'; } \
	*p++ = '\\'; *p++ = 'n'

#define FC_LINEBREAK \
	if (!MY_HasFlag(eafFlags, EAF_FC_RealLf)) \
	{ *p++ = '\r'; } *p++ = '\n'


typedef struct MyForCodingState {
	CHAR lastChar;
	SIZE_T cbExtra;
	SIZE_T nLnBreak;
} MyForCodingState_t;

typedef struct MyForCoding {
	UINT eafFlags;
	MyForCodingState_t state;
	BOOL const *pLastCharNonAscii;
} MyForCoding_t;

static BOOL MyFC_Count(
	MyForCoding_t *pFC, CHAR const c
);

static void MyFC_AddCb(
	MyForCoding_t const *pFC, SIZE_T *pcb
);

static BOOL MyFC_Mutate(
	MyForCoding_t *pFC, CHAR **ppCurr,
	CHAR const cNext
);


/* ............................. ForCoding declare */


static HRESULT MyBase_ToAscii(
	IChar_t const *pszSrc,
	SIZE_T nSrc,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	BOOL const dbcsConti = MY_HasFlag(eafFlags, EAF_DbcsConti);
	BOOL const forCoding = MY_HasFlag(eafFlags, EAF_ForCoding);
	HRESULT hr = 0;
	BOOL isDbcsConti = FALSE;
	BOOL lastCharNonAscii = FALSE;
	MyForCoding_t fc = { 0 };
	SIZE_T i = 0, cbAscii = 0;
	LPSTR pszAscii = NULL;
	CHAR *p = NULL;

	if (!pszSrc || (!ppszAscii && !pcbAscii)) {
		hr = E_INVALIDARG; goto eof;
	}
	
	fc.eafFlags = eafFlags;
	fc.pLastCharNonAscii = &lastCharNonAscii;
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
			CHAR const ascii = (c & 0xFF);
			BOOL handled = FALSE;
			if (forCoding) {
				handled = MyFC_Count(&fc, ascii);
			}
			if (!handled) {
				cbAscii += sizeof(CHAR);
			}
			lastCharNonAscii = FALSE;
			continue;
		}
		else if (dbcsConti) {
			isDbcsConti = TRUE;
		}
		cbAscii += _ESCASCII_NonAsciiCB;
		lastCharNonAscii = TRUE;
	}
	if (forCoding) {
		MyFC_AddCb(&fc, &cbAscii);
	}
	if (!ppszAscii) goto eof;

	pszAscii = MyAllocZero(cbAscii + 1);
	if (!pszAscii) {
		hr = E_OUTOFMEMORY; goto eof;
	}
	isDbcsConti = FALSE;
	p = pszAscii;
	if (forCoding) {
		MyForCodingState_t *ps = &fc.state;
		ZeroMemory(ps, sizeof(*ps));
		FC_DQUOTE_ON;
	}
	for (i = 0; i < nSrc; ++i)
	{
		IChar_t const c = pszSrc[i];
		if (isDbcsConti) {
			isDbcsConti = FALSE;
		}
		else if ((unsigned)c < 0x80) {
			CHAR const ascii = (c & 0xFF);
			BOOL handled = FALSE;
			if (forCoding) {
				handled = MyFC_Mutate(&fc, &p, ascii);
			}
			if (!handled) {
				*p++ = ascii;
			}
			lastCharNonAscii = FALSE;
			continue;
		}
		else if (dbcsConti) {
			isDbcsConti = TRUE;
		}
		*p++ = '\\';
		*p++ = 'x';
		MyAppendHex(&p, c);
		lastCharNonAscii = TRUE;
	}
	if (forCoding) {
		FC_DQUOTE_OFF;
	}
	*ppszAscii = pszAscii;
eof:
	if (pcbAscii) { *pcbAscii = cbAscii; }
	return hr;
}



/* ForCoding define  ............................. */

static BOOL MyFC_Count(
	MyForCoding_t *pFC, CHAR const c)
{
	BOOL const lastCharNonAscii = *(pFC->pLastCharNonAscii);
	MyForCodingState_t *ps = &pFC->state;
	if (c == '\r') {
		++(ps->nLnBreak);
		ps->lastChar = c;
		return TRUE;
	}
	if (c == '\n') {
		/* avoid treat CrLf twice */
		if (ps->lastChar != '\r') {
			++(ps->nLnBreak);
		}
		ps->lastChar = c;
		return TRUE;
	}
	if (c == '\\') {
		ps->cbExtra += 2;
		return TRUE;
	}
	if (c == '\"') {
		ps->cbExtra += 2;
		return TRUE;
	}
	if (c >= '0' && c <= '9') {
		if (lastCharNonAscii) {
			/* DquoteOff, Space, DquoteOn, 'c' */
			ps->cbExtra += FC_DQUOTE_PAIR_CB + 2;
			return TRUE;
		}
	}
	return FALSE;
}

static void MyFC_AddCb(
	MyForCoding_t const *pFC, SIZE_T *pcb)
{
	UINT const eafFlags = pFC->eafFlags;
	MyForCodingState_t const *ps = &pFC->state;
	SIZE_T const nLnBrk = ps->nLnBreak;
	SIZE_T const nLine = (ps->nLnBreak) + 1;
	
	SIZE_T const cbCodeNL =
		MY_HasFlag(eafFlags, EAF_FC_CodeCrLf) ?
		(nLnBrk * 4) : (nLnBrk * 2);
	
	SIZE_T const cbRealNL =
		MY_HasFlag(eafFlags, EAF_FC_RealLf) ?
		(nLnBrk) : (nLnBrk * 2);
	
	*pcb += (FC_DQUOTE_PAIR_CB * nLine)
		+ cbCodeNL + cbRealNL + (ps->cbExtra);
}

static BOOL MyFC_Mutate(
	MyForCoding_t *pFC, CHAR **ppCurr,
	CHAR const cNext)
{
	UINT const eafFlags = pFC->eafFlags;
	BOOL const lastCharNonAscii = *(pFC->pLastCharNonAscii);
	MyForCodingState_t *ps = &pFC->state;
	
	BOOL handled = FALSE;
	CHAR *p = *ppCurr;
	BOOL wantMut = FALSE;
	
	if (cNext == '\r') {
		handled = TRUE;
		wantMut = TRUE;
		ps->lastChar = cNext;
		goto eof;
	}
	if (cNext == '\n') {
		handled = TRUE;
		/* avoid treat CrLf twice */
		if (ps->lastChar != '\r') {
			wantMut = TRUE;
		}
		ps->lastChar = cNext;
		goto eof;
	}
	if (cNext == '\\') {
		handled = TRUE;
		*p++ = '\\';
		*p++ = '\\';
		goto eof;
	}
	if (cNext == '\"') {
		handled = TRUE;
		*p++ = '\\';
		*p++ = '\"';
		goto eof;
	}
	if (cNext >= '0' && cNext <= '9') {
		if (lastCharNonAscii) {
			handled = TRUE;
			FC_DQUOTE_OFF;
			*p++ = ' ';
			FC_DQUOTE_ON;
			*p++ = cNext;
			goto eof;
		}
	}
eof:
	if (wantMut) {
		FC_LINEBREAKCODE;
		FC_DQUOTE_OFF;
		FC_LINEBREAK;
		FC_DQUOTE_ON;
	}
	*ppCurr = p;
	return handled;
}

/* .............................  ForCoding define */
