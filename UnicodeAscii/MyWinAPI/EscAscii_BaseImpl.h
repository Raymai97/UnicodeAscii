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

#define MY_IsHexChar(c) ( \
	((c) >= '0' && (c) <= '9') || \
	((c) >= 'a' && (c) <= 'f') || \
	((c) >= 'A' && (c) <= 'F') )


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

#define FC_DQUOTE_ON		_ESCASCII_DquoteOn
#define FC_DQUOTE_OFF		_ESCASCII_DquoteOff
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
	BOOL const *pLastCharNonAscii;
	MyForCodingState_t state;
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

static HRESULT MyEscapeToAscii(
	IChar_t const *pszSrc,
	SIZE_T const nSrc,
	UINT const eafFlags,
	LPSTR pszAscii,
	SIZE_T *pcbAscii)
{
	BOOL const dbcsConti = MY_HasFlag(eafFlags, EAF_DbcsConti);
	BOOL const forCoding = MY_HasFlag(eafFlags, EAF_ForCoding);
	CHAR *p = pszAscii;
	SIZE_T i = 0, cbAscii = 0;
	BOOL isDbcsConti = FALSE;
	BOOL lastCharNonAscii = FALSE;
	MyForCoding_t fc = { 0 };
	fc.eafFlags = eafFlags;
	fc.pLastCharNonAscii = &lastCharNonAscii;

	if (forCoding) {
		if (p) { FC_DQUOTE_ON; }
	}
	for (i = 0; i < nSrc; ++i)
	{
		IChar_t const c = pszSrc[i];
		if /* Prev char is non-ASCII */
			(isDbcsConti)
		{
			/* Curr char = DBCS continuous byte */
			isDbcsConti = FALSE;
		}
		else if /* Curr char is ASCII */
			((unsigned)c < 0x80)
		{
			CHAR const ascii = (c & 0xFF);
			BOOL handled = FALSE;
			/*
				For coding, we need to handle more, such as
				forward-slash, double-quote, CrLf, etc.
			*/
			if (forCoding) {
				handled = p
					? MyFC_Mutate(&fc, &p, ascii)
					: MyFC_Count(&fc, ascii);
			}
			/*
				Otherwise, use the ASCII char as-is.
			*/
			if (!handled) {
				if (p) { *p++ = ascii; }
				else { cbAscii += sizeof(CHAR); }
			}
			lastCharNonAscii = FALSE;
			continue;
		}
		else /* Curr char is non-ASCII */
			if (dbcsConti)
		{
			/* Next char = DBCS continuous byte */
			isDbcsConti = TRUE;
		}
		/*
			Escape curr char to \x1234 form.
		*/
		if (p) {
			*p++ = '\\';
			*p++ = 'x';
			MyAppendHex(&p, c);
		}
		else {
			cbAscii += _ESCASCII_NonAsciiCB;
		}
		lastCharNonAscii = TRUE;
	}
	if (forCoding) {
		if (p) { FC_DQUOTE_OFF; }
		else { MyFC_AddCb(&fc, &cbAscii); }
	}
	if (pcbAscii) {
		*pcbAscii = cbAscii;
	}
	return S_OK;
}


static HRESULT MyBase_EscapeToAscii(
	IChar_t const *pszSrc,
	SIZE_T nSrc,
	UINT eafFlags,
	LPSTR *ppszAscii,
	SIZE_T *pcbAscii)
{
	HRESULT hr = 0;
	SIZE_T cbAscii = 0;
	LPSTR pszAscii = NULL;
	if (!pszSrc || (!ppszAscii && !pcbAscii)) {
		hr = E_INVALIDARG; goto eof;
	}

	/* Assume strlen if nSrc not specified. */
	if (nSrc == 0) {
		nSrc = MyStrLen(pszSrc);
	}

	/* Find cbNeeded for the output. */
	hr = MyEscapeToAscii(pszSrc, nSrc, eafFlags, NULL, &cbAscii);
	if (FAILED(hr)) goto eof;
	if (pcbAscii) { *pcbAscii = cbAscii; }
	if (!ppszAscii) goto eof;

	/* Alloc-Write the output. */
	pszAscii = MyAllocZero(cbAscii + 1);
	if (!pszAscii) {
		hr = E_OUTOFMEMORY; goto eof;
	}
	hr = MyEscapeToAscii(pszSrc, nSrc, eafFlags, pszAscii, NULL);
	if (FAILED(hr)) goto eof;
	if (ppszAscii) { *ppszAscii = pszAscii; }
eof:
	if (FAILED(hr)) {
		if (pszAscii) { MyFree(pszAscii); }
	}
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
	if (lastCharNonAscii && MY_IsHexChar(c)) {
		/* DquoteOff, Space, DquoteOn, 'c' */
		ps->cbExtra += FC_DQUOTE_PAIR_CB + 2;
		return TRUE;
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
	BOOL wantLnBrk = FALSE;
	
	if (cNext == '\r') {
		handled = TRUE;
		wantLnBrk = TRUE;
		ps->lastChar = cNext;
		goto eof;
	}
	if (cNext == '\n') {
		handled = TRUE;
		/* avoid treat CrLf twice */
		if (ps->lastChar != '\r') {
			wantLnBrk = TRUE;
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
	if (lastCharNonAscii && MY_IsHexChar(cNext)) {
		handled = TRUE;
		FC_DQUOTE_OFF;
		*p++ = ' ';
		FC_DQUOTE_ON;
		*p++ = cNext;
		goto eof;
	}
eof:
	if (wantLnBrk) {
		FC_LINEBREAKCODE;
		FC_DQUOTE_OFF;
		FC_LINEBREAK;
		FC_DQUOTE_ON;
	}
	*ppCurr = p;
	return handled;
}

/* .............................  ForCoding define */
