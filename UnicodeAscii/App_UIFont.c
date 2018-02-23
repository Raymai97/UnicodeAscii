#include "App.h"

#ifndef UNICODE
#define A_OR_W(a, w)		a
#else
#define A_OR_W(a, w)		w
#endif



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
