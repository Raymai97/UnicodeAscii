#pragma once
#include "MainWin.h"

struct MainWin {
	HWND hwndSelf;
	HWND hwndFocus;
	HFONT hfoEnglish;
	HFONT hfoEditBox;
};

#define ENCODING_TEXT(i) (\
	i == EncodingWide ? TEXT("Windows Unicode") : \
	i == EncodingUTF8 ? TEXT("UTF-8") : \
	i == EncodingBig5 ? TEXT("Big5 (Taiwan)") : \
	i == EncodingGBK ? TEXT("GBK (China)") : \
	i == EncodingShiftJIS ? TEXT("Shift-JIS (Japan)") : \
	i == Encoding437 ? TEXT("DOS-437") : \
	NULL)

enum MyEncoding {
	EncodingWide = 0,
	EncodingUTF8,
	EncodingBig5,
	EncodingGBK,
	EncodingShiftJIS,
	Encoding437,
	FirstEncoding = EncodingWide,
	LastEncoding = Encoding437,
	FirstDbcsEncoding = EncodingBig5,
	LastDbcsEncoding = EncodingShiftJIS
};

enum MyChildId {
	FIRST_ChildId = 9,
	cboOutputEnc,
	edtInput,
	btnCopyASCII
};

static LPCTSTR const
s_pszSelfClass = APP_Name TEXT(".MainWin"),
s_pszSelfTitle = APP_Name;


EXTERN_C void MainWin_OnLoad(MainWin_t *pSelf);

EXTERN_C void MainWin_OnUnload(MainWin_t *pSelf);

EXTERN_C void MainWin_OnCommand(
	MainWin_t *pSelf, int id, int nCode, HWND hCtl
);
