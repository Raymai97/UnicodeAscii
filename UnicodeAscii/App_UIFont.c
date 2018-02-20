#include "App.h"

#ifndef UNICODE
#define A_OR_W(a, w)		a
#else
#define A_OR_W(a, w)		w
#endif


static LPCTSTR const
s_Arial = TEXT(
	"Arial"
),
s_Tahoma = TEXT(
	"Tahoma"
),
s_SegoeUI = TEXT(
	"Segoe UI"
),
s_MS_ShellDlg = TEXT(
	"MS Shell Dlg"
),
s_MS_ShellDlg2 = TEXT(
	"MS Shell Dlg 2"
),
s_MS_PGothic = A_OR_W(
	"\x82\x6C\x82\x72 \x82\x6F\x83\x53\x83\x56\x83\x62\x83\x4E",
	L"\xFF2D\xFF33 \xFF30\x660E\x671D"
),
s_SimSun = A_OR_W(
	"\xCB\xCE\xCC\xE5",
	L"\x5B8B\x4F53"
),
s_PMingLiu = A_OR_W(
	"\xB7\x73\xB2\xD3\xA9\xFA\xC5\xE9",
	L"\x65B0\x7D30\x660E\x9AD4"
);

typedef struct {
	UINT    cbSize;
	int     iBorderWidth;
	int     iScrollWidth;
	int     iScrollHeight;
	int     iCaptionWidth;
	int     iCaptionHeight;
	LOGFONT lfCaptionFont;
	int     iSmCaptionWidth;
	int     iSmCaptionHeight;
	LOGFONT lfSmCaptionFont;
	int     iMenuWidth;
	int     iMenuHeight;
	LOGFONT lfMenuFont;
	LOGFONT lfStatusFont;
	LOGFONT lfMessageFont;
} NonClientMetrics_t;


EXTERN_C HFONT App_CreateEditBoxFont(void)
{
	NonClientMetrics_t ncm = { sizeof(ncm) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	return CreateFontIndirect(&(ncm.lfMessageFont));
}

EXTERN_C HFONT App_CreateEnglishFont(void)
{
	HFONT hfo = NULL;
	hfo = App_CreateFont(s_SegoeUI, 9);
	if (hfo) goto eof;
	hfo = App_CreateFont(s_Tahoma, 8);
	if (hfo) goto eof;
	hfo = App_CreateFont(s_Arial, 8);
eof:
	return hfo;
}
