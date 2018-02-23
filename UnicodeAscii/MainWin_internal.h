#pragma once
#include "MainWin.h"

struct MainWin {
	HWND hwndSelf;
	HWND hwndFocus;
	HFONT hfoEnglish;
	LOGFONT lfEditBox;
};

static LPCTSTR const
s_pszSelfClass = TEXT("MainWin");

EXTERN_C void MainWin_OnLoad(MainWin_t *pSelf);

EXTERN_C void MainWin_OnUnload(MainWin_t *pSelf);

EXTERN_C void MainWin_OnCommand(
	MainWin_t *pSelf, int id, int nCode, HWND hCtl
);
