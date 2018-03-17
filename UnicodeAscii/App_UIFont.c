#include "App.h"

#ifndef UNICODE
#define A_OR_W(a, w)		a
#else
#define A_OR_W(a, w)		w
#endif


EXTERN_C BOOL App_CreateEditBoxLogFont(LOGFONT *pLF)
{
/*
	For Win3.x OS, just return FALSE to not create font, so that
	'System' font will be used, and text-zoom will be disabled.
	This gurantees proper text-rendering and best look-and-feel
	especially on east-asian Win3.1.
*/
#ifndef _WIN64
	if (OSGetOSMajorVer() < 4) {
		return FALSE;
	}
#endif
	if (OSQueryMessageFont(pLF)) {
		/* Use default height value */
		pLF->lfHeight = 0;
		/* Force True-Type sans-serif */
		pLF->lfOutPrecision = OUT_TT_ONLY_PRECIS;
		pLF->lfPitchAndFamily = FF_SWISS;
		return TRUE;
	}
	return FALSE;
}

EXTERN_C HFONT App_CreateEnglishFont(void)
{
	App_FontInfo_t const
		segoeUI = { "Segoe UI", 9 },
		tahoma = { "Tahoma", 8 },
		arial = { "MS Sans Serif", 8 };
	HFONT hfo = NULL;
	hfo = App_CreateFont(&segoeUI);
	if (hfo) goto eof;
	hfo = App_CreateFont(&tahoma);
	if (hfo) goto eof;
	hfo = App_CreateFont(&arial);
eof:
	return hfo;
}
