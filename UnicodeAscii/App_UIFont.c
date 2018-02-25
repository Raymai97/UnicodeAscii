#include "App.h"

#ifndef UNICODE
#define A_OR_W(a, w)		a
#else
#define A_OR_W(a, w)		w
#endif


EXTERN_C BOOL App_CreateEditBoxLogFont(LOGFONT *pLF)
{
/*
	In non-English Windows before Win4.0, SPI returns wrong font.
	It will return MS Sans Serif even on asian edition of Windows.
*/
#ifndef _WIN64
	if (OSGetOSMajorVer() < 4)
	{
		HFONT hfoSys = NULL;
		BYTE charsetSys = 0;
		hfoSys = (HFONT)GetStockObject(SYSTEM_FONT);
		GetObject(hfoSys, sizeof(*pLF), pLF);
		charsetSys = pLF->lfCharSet;
		ZeroMemory(pLF, sizeof(*pLF));
		pLF->lfCharSet = charsetSys;
		pLF->lfOutPrecision = OUT_OUTLINE_PRECIS;
		return TRUE;
	}
#endif
	return OSQueryMessageFont(pLF);
}

EXTERN_C HFONT App_CreateEnglishFont(void)
{
	App_FontInfo_t const
		segoeUI = { "Segoe UI", 9 },
		tahoma = { "Tahoma", 8 },
		arial = { "Arial", 8 };
	HFONT hfo = NULL;
	hfo = App_CreateFont(&segoeUI);
	if (hfo) goto eof;
	hfo = App_CreateFont(&tahoma);
	if (hfo) goto eof;
	hfo = App_CreateFont(&arial);
eof:
	return hfo;
}
