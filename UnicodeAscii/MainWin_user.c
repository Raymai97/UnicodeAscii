#include "MainWin_internal.h"
#include <MyWinAPI/EscAscii.h>

#define MY_SafeFree		APP_SafeFree


/*
	If Active CP is DBCS like Shift-JIS,
	*pCC = char count, return TRUE.
*/
static BOOL MyMbsCharCount(LPCTSTR psz, UINT *pCC)
{
#ifndef UNICODE
	CPINFO cpi = { 0 };
	UINT i = 0, cc = 0;
	BOOL isDbcsConti = FALSE;
	GetCPInfo(GetACP(), &cpi);
	if (cpi.MaxCharSize < 2) {
		return FALSE;
	}
	for (i = 0; psz[i]; ++i, ++cc) {
		CHAR const c = psz[i];
		if (isDbcsConti) {
			isDbcsConti = FALSE;
			--cc;
		}
		else if ((unsigned)c >= 0x80) {
			isDbcsConti = TRUE;
		}
	}
	*pCC = cc;
	return TRUE;
#else /*UNICODE*/
	UNREFERENCED_PARAMETER(psz);
	UNREFERENCED_PARAMETER(pCC);
	return FALSE;
#endif
}

/*
	Locate/highlight the first unrepresentable char in Input textbox.
	pszSad ended right before unrepresentable char.
*/
static BOOL MyLocateBadChar(
	HWND hwnd, LPCSTR pszSad, UINT cpSad)
{
	BOOL ok = FALSE;
	LPTSTR pszSAD = NULL;
	UINT iBadCh = 0;
	HWND hEdt = NULL;

	OSAllocTStrFromMulti(&pszSAD, cpSad, pszSad, -1);
	if (pszSAD && !MyMbsCharCount(pszSAD, &iBadCh))
	{
		for (iBadCh = 0; pszSAD[iBadCh]; ++iBadCh);
	}
	hEdt = GetDlgItem(hwnd, edtInput);
	SetForegroundWindow(hwnd);
	SetFocus(hEdt);
	SendMessage(hEdt, EM_SETSEL, iBadCh, iBadCh + 1);

	MY_SafeFree(pszSAD, MemFree);
	return ok;
}

static void MyTask_CopyAscii(MainWin_t *pSelf)
{
	HWND const hwnd = pSelf->hwndSelf;
	HRESULT hr = 0;
	int iOutEnc = 0;
	LPWSTR pszInW = NULL;
	int cchInW = 0;
	LPSTR pszInA = NULL;
	UINT eafFlags = 0;
	LPSTR pszOut = NULL;
	SIZE_T cbOut = 0;

	iOutEnc = (int)SendMessage(GetDlgItem(hwnd, cboOutputEnc), CB_GETCURSEL, 0, 0);
	if (iOutEnc < FirstEncoding || iOutEnc > LastEncoding) {
		MessageBoxA(hwnd, "Invalid encoding!", NULL, MB_ICONERROR);
		goto eof;
	}
	
	hr = OSGetHwndTextW(GetDlgItem(hwnd, edtInput), &pszInW, &cchInW);
	if (FAILED(hr)) {
		MessageBoxA(hwnd, "OSGetHwndTextW() failed!", NULL, MB_ICONERROR);
		goto eof;
	}
	if (!pszInW) {
		MessageBoxA(hwnd, "No input!", NULL, MB_ICONERROR);
		goto eof;
	}
	
	if (iOutEnc == EncodingWide)
	{
		hr = EscAscii_FromWide(pszInW, cchInW, eafFlags, &pszOut, &cbOut);
		if (FAILED(hr)) {
			MessageBoxA(hwnd, "EscAscii_FromWide() failed!", NULL, MB_ICONERROR);
			goto eof;
		}
	}
	else /* Multibyte Encoding */
	{
		UINT const cp =
			(iOutEnc == EncodingUTF8) ? CP_UTF8 :
			(iOutEnc == Encoding437) ? 437 :
			(iOutEnc == EncodingBig5) ? 950 :
			(iOutEnc == EncodingGBK) ? 936 :
			(iOutEnc == EncodingShiftJIS) ? 932 : -1;

		hr = OSAllocMultiFromWide(&pszInA, cp, pszInW, -1);
		if (hr == S_UNREPRESENTABLE_CHAR) {
			MessageBoxA(hwnd, "Found unpresentable char!", NULL, MB_ICONERROR);
			MyLocateBadChar(hwnd, pszInA, cp);
			goto eof;
		}
		if (FAILED(hr)) {
			LPCSTR pszErr =
				hr == E_CODEPAGE_NOT_SUPPORTED ?
				"Encoding not supported by current OS!" :
				"OSAllocMultiFromWide() failed!";
			MessageBoxA(hwnd, pszErr, NULL, MB_ICONERROR);
			goto eof;
		}
		/* ... done preparing pszInA */
		if (iOutEnc >= FirstDbcsEncoding && iOutEnc <= LastDbcsEncoding)
		{
			eafFlags |= EAF_DbcsConti;
		}
		hr = EscAscii_FromMulti(pszInA, 0, eafFlags, &pszOut, &cbOut);
		if (FAILED(hr)) {
			MessageBoxA(hwnd, "EscAscii_FromMulti() failed!", NULL, MB_ICONERROR);
			goto eof;
		}
	}
	/* ... done preparing pszOut */
	hr = OSSetClipboardTextA(pszOut, cbOut);
	if (FAILED(hr)) {
		MessageBoxA(hwnd, "Failed to set clipboard text!", NULL, MB_ICONERROR);
		goto eof;
	}
	MessageBoxA(hwnd, "OK! ASCII copied to Clipboard!", "OK!", MB_ICONINFORMATION);
eof:
	MY_SafeFree(pszInW, MemFree);
	MY_SafeFree(pszInA, MemFree);
	MY_SafeFree(pszOut, MemFree);
}


static void MyInitSysMenu(HWND hwnd)
{
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MFS_DISABLED, 0,
		APP_Name TEXT(" by raymai97"));
	AppendMenu(hMenu, MFS_DISABLED, 0, TEXT("v1.0 ")
#ifndef UNICODE
		"ANSI Build"
#else
		L"Unicode Build"
#endif
	);
}


EXTERN_C void MainWin_OnLoad(MainWin_t *pSelf)
{
	HWND const hwnd = pSelf->hwndSelf;
	HFONT hfoEnglish = NULL;
	HFONT hfoEditBox = NULL;
	HWND hCtl = NULL;
	int x = 0, y = 0, cx = 0, cy = 0, i = 0;

	MyInitSysMenu(hwnd);
	App_SetWindowClientSize(hwnd, cx = 420, cy = 250);

	hfoEnglish = App_CreateEnglishFont();
	pSelf->hfoEnglish = hfoEnglish;
	hfoEditBox = App_CreateEditBoxFont();
	pSelf->hfoEditBox = hfoEditBox;

	hCtl = App_CreateChild(hwnd, 0, WC_STATIC,
		TEXT("Convert Unicode string to ASCII-compatible ")
		TEXT("string-literal for C/C++."),
		x = 15, y = 15, cx = 390, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	hCtl = App_CreateChild(hwnd, 0, WC_STATIC,
		TEXT("Input:"),
		x = 20, y = 50, cx = 45, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX |
		SS_RIGHT, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	hCtl = App_CreateChild(hwnd, edtInput, WC_EDIT, NULL,
		x = 73, y - 1, cx = 330, cy = 120,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL |
		ES_WANTRETURN | ES_MULTILINE,
		WS_EX_CLIENTEDGE);
	SetWindowLongPtr(hCtl, GWLP_WNDPROC,
		(LONG_PTR)App_CtlWndProc_Edit);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEditBox, FALSE);
	SetFocus(hCtl);

	hCtl = App_CreateChild(hwnd, 0, WC_STATIC,
		TEXT("Output:"),
		x = 20, y = 190, cx = 45, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX |
		SS_RIGHT, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	hCtl = App_CreateChild(hwnd, cboOutputEnc, WC_COMBOBOX, NULL,
		x = 73, y - 1, cx = 160, cy = 200,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		CBS_DROPDOWNLIST, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	for (i = FirstEncoding; i <= LastEncoding; ++i)
	{
		SendMessage(hCtl, CB_ADDSTRING, 0, (LPARAM)ENCODING_TEXT(i));
	}
	SendMessage(hCtl, CB_SETCURSEL, 0, 0);

	hCtl = App_CreateChild(hwnd, btnCopyASCII, WC_BUTTON,
		TEXT("Copy ASCII to Clipboard"),
		x = 235, y - 2, cx = 168, cy = 27,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
}

EXTERN_C void MainWin_OnUnload(MainWin_t *pSelf)
{
	MY_SafeFree(pSelf->hfoEnglish, DeleteObject);
	MY_SafeFree(pSelf->hfoEditBox, DeleteObject);
}

EXTERN_C void MainWin_OnCommand(
	MainWin_t *pSelf, int id, int nCode, HWND hCtl)
{
	UNREFERENCED_PARAMETER(nCode);
	UNREFERENCED_PARAMETER(hCtl);
	if (id == btnCopyASCII)
	{
		MyTask_CopyAscii(pSelf);
	}
}
