#pragma once
#include "App.h"

/* ZoomIn  = [Ctrl] & ([+] or mouse-wheel up) */
/* ZoomOut = [Ctrl] & ([-] or mouse-wheel down) */
/* ZoomReset = [Ctrl] & [0] */

enum AppEditBox_MsgCode {
	EM_First = WM_APP,
	/* (WPARAM)(BOOL)redraw */
	/* (LPARAM)(LOGFONT const *) */
	EM_SetFontCopy,
	/* (WPARAM)(BOOL)enableZoom */
	/* (LPARAM)(App_FontZoomSpec_t const *) */
	EM_SetFontZoomSpec
};



EXTERN_C void AppEditBox_Init(void);

EXTERN_C LRESULT CALLBACK AppEditBox_WndProc(
	HWND hCtl, UINT msg, WPARAM w, LPARAM l
);

