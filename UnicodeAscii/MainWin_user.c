#include "MainWin_internal.h"
#include "AppEditBox.h"
#include <MyWinAPI/EscAscii.h>
#include <MyWinAPI/StrOfInt.h>

#define MY_SafeFree		APP_SafeFree

enum MyChildId {
	cboOutputEnc = 10,
	edtInput,
	edtHidden,
	btnCopyASCII,
	chkForCoding,
	chkFcCodeCrLf,
	chkFcRealLf
};

enum MyEncoding {
	EncodingWide = 0,
	EncodingUTF8,
	EncodingLatin1,
	EncodingBig5,
	EncodingGBK_2312,
	EncodingShiftJIS,
	EncodingMAX,
	FirstDbcsEncoding = EncodingBig5,
	LastDbcsEncoding = EncodingShiftJIS
};

typedef struct MyEncoInfo MyEncoInfo_t;
struct MyEncoInfo {
	UINT mbCodepage;
	LPCSTR pszTextA;
};

static MyEncoInfo_t s_EncoInfos[EncodingMAX];


static void MyInitEncoInfos(void);
static void MyInitSysMenu(HWND hwnd);
static void MyInitEditBox(HWND hCtl, LOGFONT const *pLF);

static void MyMsgErr(HWND hwnd, HRESULT hr, LPCSTR pszMsg);

static BOOL MyLocateBadChar(HWND hwnd, LPCSTR pszSad, UINT cpSad);
static void MyTask_CopyAscii(MainWin_t *pSelf);
static void MyOnChkForCoding(MainWin_t *pSelf);



EXTERN_C void MainWin_OnLoad(MainWin_t *pSelf)
{
	HWND const hwnd = pSelf->hwndSelf;
	HFONT hfoEnglish = NULL;
	BOOL hasEditBoxFont = FALSE;
	HWND hCtl = NULL;
	int x = 0, y = 0, cx = 0, cy = 0, i = 0;

	MyInitEncoInfos();
	MyInitSysMenu(hwnd);
	App_SetWindowClientSize(hwnd, cx = 420, cy = 250);
	SetWindowTextA(hwnd, APP_Name " " APP_VerStr);

	hfoEnglish = App_CreateEnglishFont();
	pSelf->hfoEnglish = hfoEnglish;
	hasEditBoxFont = App_CreateEditBoxLogFont(&pSelf->lfEditBox);
	
	hCtl = App_CreateChild(hwnd, 0, WC_STATIC, NULL,
		x = 15, y = 15, cx = 390, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
	SetWindowTextA(hCtl,
		"Convert Unicode string to ASCII-compatible "
		"string-literal for C/C++.");

	hCtl = App_CreateChild(hwnd, 0, WC_STATIC, NULL,
		x = 20, y = 50, cx = 45, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX |
		SS_RIGHT, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
	SetWindowTextA(hCtl, "&Input:");

	hCtl = App_CreateChild(hwnd, edtInput, WC_EDIT, NULL,
		x = 73, y - 1, cx = 330, cy = 120,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL |
		ES_NOHIDESEL | ES_MULTILINE | ES_WANTRETURN,
		WS_EX_CLIENTEDGE);
	MyInitEditBox(hCtl, hasEditBoxFont ? &pSelf->lfEditBox : NULL);
	SetFocus(hCtl);

	hCtl = App_CreateChild(hwnd, edtHidden, WC_EDIT, NULL,
		0, 0, 0, 0, WS_CHILD | ES_MULTILINE, 0);

	hCtl = App_CreateChild(hwnd, 0, WC_STATIC, NULL,
		x = 20, y = 190, cx = 45, cy = 20,
		WS_CHILD | WS_VISIBLE | WS_DEBUGBOX |
		SS_RIGHT, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
	SetWindowTextA(hCtl, "&Output:");

	hCtl = App_CreateChild(hwnd, cboOutputEnc, WC_COMBOBOX, NULL,
		x = 73, y = 189, cx = 160, cy = 120,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
		CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT, 0);
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	for (i = 0; i < EncodingMAX; ++i)
	{
		LPCSTR const pszTextA = s_EncoInfos[i].pszTextA;
		SendMessageA(hCtl, CB_ADDSTRING, 0, (LPARAM)pszTextA);
	}
	SendMessage(hCtl, CB_SETCURSEL, 0, 0);

	hCtl = App_CreateChild(hwnd, chkForCoding, WC_BUTTON, NULL,
		x = 73, y = 220, cx = 90, cy = 18,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_AUTOCHECKBOX, 0);
	SetWindowTextA(hCtl, "&For coding");
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
	SendMessage(hCtl, BM_SETCHECK, BST_CHECKED, 0);

	hCtl = App_CreateChild(hwnd, chkFcCodeCrLf, WC_BUTTON, NULL,
		x = 168, y = 220, cx = 110, cy = 18,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_AUTOCHECKBOX, 0);
	SetWindowTextA(hCtl, "&Escape as \\r\\n");
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	hCtl = App_CreateChild(hwnd, chkFcRealLf, WC_BUTTON, NULL,
		x = 280, y = 220, cx = 120, cy = 18,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_AUTOCHECKBOX, 0);
	SetWindowTextA(hCtl, "&Line break with \\n");
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);

	hCtl = App_CreateChild(hwnd, btnCopyASCII, WC_BUTTON, NULL,
		x = 235, y = 188, cx = 168, cy = 27,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0);
	SetWindowTextA(hCtl, "&Copy ASCII to Clipboard");
	SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoEnglish, FALSE);
}

EXTERN_C void MainWin_OnUnload(MainWin_t *pSelf)
{
	MY_SafeFree(pSelf->hfoEnglish, DeleteObject);
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
	else if (id == chkForCoding)
	{
		MyOnChkForCoding(pSelf);
	}
}


static void MyInitEncoInfos(void)
{
	MyEncoInfo_t *p, *pEncs = s_EncoInfos;

	p = &pEncs[EncodingWide];
	p->pszTextA = "Windows Unicode";

	p = &pEncs[EncodingUTF8];
	p->mbCodepage = CP_UTF8;
	p->pszTextA = "UTF-8";

	p = &pEncs[EncodingLatin1];
	p->mbCodepage = CP_Latin1;
	p->pszTextA = "Latin-1 (CP1252)";

	p = &pEncs[EncodingBig5];
	p->mbCodepage = CP_Big5;
	p->pszTextA = "Big5 (Taiwan)";

	p = &pEncs[EncodingGBK_2312];
	p->mbCodepage = CP_GBK_2312;
	p->pszTextA = OSSupportGBK() ? "GBK (China)" : "GB2312 (China)";

	p = &pEncs[EncodingShiftJIS];
	p->mbCodepage = CP_ShiftJIS;
	p->pszTextA = "Shift-JIS (Japan)";
}

static void MyInitSysMenu(HWND hwnd)
{
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA(hMenu, MFS_DISABLED, 0,
		APP_Name " by raymai97");
	AppendMenuA(hMenu, MFS_DISABLED, 0, APP_VerStr
#ifndef UNICODE
		" ANSI Build"
#else
		" Unicode Build"
#endif
	);
}

static void MyInitEditBox(HWND hCtl, LOGFONT const *pLF)
{
	SetWindowLongPtr(hCtl, GWLP_WNDPROC, (LONG_PTR)AppEditBox_WndProc);
	if (pLF) {
		App_FontZoomSpec_t zs = { 0 };
		zs.defFontSize = App_FontSize_PtFromLog(pLF->lfHeight);
		zs.minFontSize = zs.defFontSize;
		zs.maxFontSize = 36;
		SendMessage(hCtl, EM_SetFontCopy, FALSE, (LPARAM)pLF);
		SendMessage(hCtl, EM_SetFontZoomSpec, TRUE, (LPARAM)&zs);
	}
}

static void MyMsgErr(HWND hwnd, HRESULT hr, LPCSTR pszMsg)
{
	CHAR szMsg[999] = { 0 }, *p = szMsg;
	CHAR const *q = NULL;

	for (q = pszMsg; *p = *q, *q; ++p, ++q);

	if (FAILED(hr))
	{
		CHAR szHex[] = "00000000";
		StrOfUint32((UINT32)hr, 16, szHex, 8);
		*p++ = '\n';
		*p++ = '\n';
		for (q = "HRESULT 0x"; *p = *q, *q; ++p, ++q);
		for (q = szHex; *p = *q, *q; ++p, ++q);
	}

	MessageBoxA(hwnd, szMsg, NULL, MB_ICONERROR);
}


static BOOL MyLocateBadChar(HWND hwnd, LPCSTR pszSad, UINT cpSad)
{
	BOOL ok = FALSE;
	HWND hHid = NULL;
	LPTSTR pszSAD = NULL;
	DWORD lenSAD = 0;
	HWND hEdt = NULL;

	hHid = GetDlgItem(hwnd, edtHidden);
	OSAllocTStrFromMulti(&pszSAD, cpSad, pszSad, -1);
	SendMessage(hHid, WM_SETTEXT, 0, (LPARAM)pszSAD);
	SendMessage(hHid, EM_SETSEL, 0, -1);
	SendMessage(hHid, EM_GETSEL, 0, (LPARAM)&lenSAD);

	hEdt = GetDlgItem(hwnd, edtInput);
	SendMessage(hEdt, EM_SETSEL, lenSAD, lenSAD + 1);
	SendMessage(hEdt, EM_SCROLLCARET, 0, 0);
	App_SetFocus(hEdt);

	MY_SafeFree(pszSAD, MemFree);
	return ok;
}


static void MyTask_CopyAscii(MainWin_t *pSelf)
{
	HWND const hwnd = pSelf->hwndSelf;
	HRESULT hr = 0;
	HWND hCboEnc = NULL;
	HWND hEdtInput = NULL;
	int iOutEnc = 0;
	LPWSTR pszInW = NULL;
	int cchInW = 0;
	LPSTR pszInA = NULL;
	UINT eafFlags = 0;
	LPSTR pszOut = NULL;
	SIZE_T cbOut = 0;

	hCboEnc = GetDlgItem(hwnd, cboOutputEnc);
	hEdtInput = GetDlgItem(hwnd, edtInput);

	iOutEnc = (int)SendMessage(hCboEnc, CB_GETCURSEL, 0, 0);
	if (iOutEnc < 0 || iOutEnc > EncodingMAX) {
		MyMsgErr(hwnd, 0, "Invalid encoding!");
		goto eof;
	}

	if (IsDlgButtonChecked(hwnd, chkForCoding)){
		eafFlags |= EAF_ForCoding;
	}
	if (IsDlgButtonChecked(hwnd, chkFcCodeCrLf)) {
		eafFlags |= EAF_FC_CodeCrLf;
	}
	if (IsDlgButtonChecked(hwnd, chkFcRealLf)) {
		eafFlags |= EAF_FC_RealLf;
	}

	hr = OSGetHwndTextW(hEdtInput, &pszInW, &cchInW);
	if (FAILED(hr)) {
		MyMsgErr(hwnd, hr, "OSGetHwndTextW() failed!");
		goto eof;
	}
	if (!pszInW) {
		MyMsgErr(hwnd, 0, "No input!");
		goto eof;
	}

	if (iOutEnc == EncodingWide)
	{
		hr = EscAscii_WideToAscii(pszInW, cchInW, eafFlags, &pszOut, &cbOut);
	}
	else /* Multibyte Encoding */
	{
		UINT const cp = s_EncoInfos[iOutEnc].mbCodepage;

		hr = OSAllocMultiFromWide(&pszInA, cp, pszInW, -1);
		if (hr == S_UNREPRESENTABLE_CHAR) {
			MyLocateBadChar(hwnd, pszInA, cp);
			MyMsgErr(hwnd, 0, "Found unpresentable char!");
			goto eof;
		}
		if (hr == E_CODEPAGE_NOT_SUPPORTED) {
			MyMsgErr(hwnd, 0, "Encoding not supported by current OS!");
			goto eof;
		}
		if (FAILED(hr)) {
			MyMsgErr(hwnd, hr, "OSAllocMultiFromWide() failed!");
			goto eof;
		}
		/* ... done preparing pszInA */
		if (iOutEnc >= FirstDbcsEncoding && iOutEnc <= LastDbcsEncoding)
		{
			eafFlags |= EAF_DbcsConti;
		}
		hr = EscAscii_MultiToAscii(pszInA, 0, eafFlags, &pszOut, &cbOut);
	}
	if (FAILED(hr)) {
		MyMsgErr(hwnd, hr, "EscAscii failed!");
		goto eof;
	}
	/* ... done preparing pszOut */
	hr = OSSetClipboardTextA(pszOut, cbOut);
	if (FAILED(hr)) {
		MyMsgErr(hwnd, hr, "Failed to set clipboard text!");
		goto eof;
	}
	MessageBoxA(hwnd, "OK! ASCII copied to Clipboard!", "OK!", MB_ICONINFORMATION);
eof:
	MY_SafeFree(pszInW, MemFree);
	MY_SafeFree(pszInA, MemFree);
	MY_SafeFree(pszOut, MemFree);
}


static void MyOnChkForCoding(MainWin_t *pSelf)
{
	HWND const hwnd = pSelf->hwndSelf;
	int bstFC = IsDlgButtonChecked(hwnd, chkForCoding);
	EnableWindow(
		GetDlgItem(hwnd, chkFcCodeCrLf),
		bstFC ? TRUE : FALSE);
	EnableWindow(
		GetDlgItem(hwnd, chkFcRealLf),
		bstFC ? TRUE : FALSE);
}

