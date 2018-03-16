#include "App.h"
#include "AppEditBox.h"

#define MY_CodepageNotSupported(cp) \
	((cp) != CP_ACP && (cp) != CP_OEMCP && !IsValidCodePage(cp))


typedef struct App App_t;
struct App {
	int dpi;
	WNDPROC dwpButton;
	WNDPROC dwpEdit;
};

static App_t s_App;

/*
	EXTEND: NONCLIENTMETRICS that compatible with legacy Windows.
*/
typedef struct NCM {
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
} NCM_t;

/*
	FED = Font Enum Data for MyFontEnumProc
*/

enum MyFED_Type {
	FED_CheckAvail = 1
};

typedef struct {
	UINT fedType;
} MyFED_Base_t;

typedef struct {
	MyFED_Base_t base;
	BOOL avail;
} MyFED_CheckAvail_t;


static int CALLBACK MyFontEnumProc(
	void const *lplf, void const *lptm,
	DWORD dwType, LPARAM user)
{
	UNREFERENCED_PARAMETER(lplf);
	UNREFERENCED_PARAMETER(lptm);
	UNREFERENCED_PARAMETER(dwType);
	BOOL done = FALSE;
	UINT const fedType = ((MyFED_Base_t*)user)->fedType;
	if (fedType == FED_CheckAvail)
	{
		((MyFED_CheckAvail_t*)user)->avail = TRUE;
		done = TRUE;
	}
	return done ? 0 : 1;
}


EXTERN_C LPVOID MemAllocZero(SIZE_T cb)
{
	LPVOID ptr = HeapAlloc(GetProcessHeap(), 0, cb);
	if (ptr) { ZeroMemory(ptr, cb); }
	return ptr;
}

EXTERN_C void MemFree(LPVOID ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}


EXTERN_C void App_Init(void)
{
	LoadLibraryA("comctl32");
	AppEditBox_Init();
	s_App.dpi = OSGetDPI();
	s_App.dwpButton = OSGetButtonDefWndProc();
	s_App.dwpEdit = OSGetEditDefWndProc();
}

EXTERN_C HFONT App_CreateFont(
	App_FontInfo_t const *pInfo)
{
	LPCSTR const pszName = pInfo->pszName;
	int const ptSize = pInfo->ptSize;
	HFONT hfo = NULL;
	MyFED_CheckAvail_t feDat = { FED_CheckAvail };
	HDC hdc0 = NULL;
	hdc0 = GetDC(NULL);
	if (!hdc0) goto eof;

	EnumFontsA(hdc0, pszName, MyFontEnumProc, (LPARAM)&feDat);
	if (!feDat.avail) goto eof;

	hfo = CreateFontA(
		App_FontSize_LogFromPt(ptSize),
		0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
		0, 0, 0, 0, pszName);
eof:
	if (hdc0) { ReleaseDC(NULL, hdc0); }
	return hfo;
}

EXTERN_C HWND App_CreateChild(
	HWND hwndParent, int id, LPCTSTR pszClass,
	LPCTSTR pszText, int x, int y, int cx, int cy,
	DWORD dwStyle, DWORD dwExStyle)
{
	int const dpi = s_App.dpi;
	return CreateWindowEx(dwExStyle, pszClass, pszText, dwStyle,
		MulDiv(x, dpi, 96), MulDiv(y, dpi, 96),
		MulDiv(cx, dpi, 96), MulDiv(cy, dpi, 96),
		hwndParent, (HMENU)(WPARAM)id, NULL, NULL);
}

EXTERN_C int App_FontSize_PtFromLog(int logSize)
{
	return -MulDiv(logSize, 72, s_App.dpi);
}

EXTERN_C int App_FontSize_LogFromPt(int ptSize)
{
	return -MulDiv(ptSize, s_App.dpi, 72);
}

EXTERN_C void App_SetFocus(HWND hCtl)
{
	HWND hwndFocus = NULL;
	WNDPROC wpFocus = NULL;
	WNDPROC wpCtl = NULL;

	hwndFocus = GetFocus();
	wpFocus = (WNDPROC)GetWindowLongPtr(hwndFocus, GWLP_WNDPROC);
	if (wpFocus == s_App.dwpButton)
	{
		DWORD dwStyle = GetWindowLong(hwndFocus, GWL_STYLE);
		dwStyle &= ~BS_DEFPUSHBUTTON;
		SendMessage(hwndFocus, BM_SETSTYLE, (WPARAM)dwStyle, TRUE);
	}

	wpCtl = (WNDPROC)GetWindowLongPtr(hCtl, GWLP_WNDPROC);
	if (wpCtl == s_App.dwpButton)
	{
		DWORD dwStyle = GetWindowLong(hCtl, GWL_STYLE);
		dwStyle |= BS_DEFPUSHBUTTON;
		SendMessage(hCtl, BM_SETSTYLE, (WPARAM)dwStyle, TRUE);
	}
	SetFocus(hCtl);
}

EXTERN_C void App_SetWindowClientSize(
	HWND hwnd, int cx, int cy)
{
	int const dpi = s_App.dpi;
	RECT rc = { 0 };
	GetWindowRect(hwnd, &rc);
	rc.right = rc.left + MulDiv(cx, dpi, 96);
	rc.bottom = rc.top + MulDiv(cy, dpi, 96);
	AdjustWindowRectEx(&rc,
		GetWindowLong(hwnd, GWL_STYLE),
		GetMenu(hwnd) != NULL,
		GetWindowLong(hwnd, GWL_EXSTYLE));
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;
	SetWindowPos(hwnd, NULL, 0, 0, cx, cy,
		SWP_NOMOVE | SWP_NOZORDER);
}


EXTERN_C HRESULT OSAllocMultiFromWide(
	LPSTR *ppszDst, UINT cpDst,
	LPCWSTR pszSrc, int cchSrc)
{
	HRESULT hr = 0;
	int cbDst = 0;
	LPSTR pszDst = NULL;

	if (!ppszDst || !pszSrc || !cchSrc) {
		hr = E_INVALIDARG; goto eof;
	}
	if (MY_CodepageNotSupported(cpDst)) {
		hr = E_CODEPAGE_NOT_SUPPORTED; goto eof;
	}
	if (cchSrc < 0) {
		for (cchSrc = 0; pszSrc[cchSrc]; ++cchSrc);
	}
	cbDst = WideCharToMultiByte(cpDst, 0, pszSrc, cchSrc,
		NULL, 0, NULL, NULL);
	if (cbDst < 1) {
		hr = APP_GetLastHr(); goto eof;
	}
	pszDst = MemAllocZero(++cbDst);
	if (!pszDst) {
		hr = E_OUTOFMEMORY; goto eof;
	}
	if (cpDst >= CP_UTF7) {
		WideCharToMultiByte(cpDst, 0, pszSrc, cchSrc,
			pszDst, cbDst, NULL, NULL);
	}
	else {
		CHAR const chNul = '\0';
		BOOL isBad = FALSE;
		WideCharToMultiByte(cpDst, 0, pszSrc, cchSrc,
			pszDst, cbDst, &chNul, &isBad);
		if (isBad) {
			hr = S_UNREPRESENTABLE_CHAR;
			/* don't goto eof */
		}
	}
	*ppszDst = pszDst;
eof:
	return hr;
}

EXTERN_C HRESULT OSAllocWideFromMulti(
	LPWSTR *ppszDst, UINT cpSrc,
	LPCSTR pszSrc, int cbSrc)
{
	HRESULT hr = 0;
	int cchDst = 0;
	LPWSTR pszDst = NULL;

	if (!ppszDst || !pszSrc || !cbSrc) {
		hr = E_INVALIDARG; goto eof;
	}
	if (MY_CodepageNotSupported(cpSrc)) {
		hr = E_CODEPAGE_NOT_SUPPORTED; goto eof;
	}
	if (cbSrc < 0) {
		for (cbSrc = 0; pszSrc[cbSrc]; ++cbSrc);
	}
	cchDst = MultiByteToWideChar(cpSrc, 0, pszSrc, cbSrc, NULL, 0);
	if (cchDst < 1) {
		hr = APP_GetLastHr(); goto eof;
	}
	pszDst = MemAllocZero((++cchDst) * sizeof(WCHAR));
	if (!pszDst) {
		hr = E_OUTOFMEMORY; goto eof;
	}
	MultiByteToWideChar(cpSrc, 0, pszSrc, cbSrc, pszDst, cchDst);
	*ppszDst = pszDst;
eof:
	return hr;
}

EXTERN_C HRESULT OSAllocTStrFromMulti(
	LPTSTR *ppszDst, UINT cpSrc,
	LPCSTR pszSrc, int cbSrc)
{
#ifndef UNICODE
	HRESULT hr = 0;
	LPWSTR pszW = NULL;
	hr = OSAllocWideFromMulti(&pszW, cpSrc, pszSrc, cbSrc);
	if (FAILED(hr)) goto eof;
	hr = OSAllocMultiFromWide(ppszDst, CP_ACP, pszW, -1);
eof:
	APP_SafeFree(pszW, MemFree);
	return hr;
#else /* UNICODE */
	return OSAllocWideFromMulti(ppszDst, cpSrc, pszSrc, cbSrc);
#endif
}

EXTERN_C BOOL OSEdit_SelectNextChar(HWND hCtl, int iStart)
{
	DWORD len = 0, iEnd = 0, iEnd2 = 0;
	len = GetWindowTextLength(hCtl);
	for (iEnd = iStart + 1; iEnd <= len; ++iEnd)
	{
		SendMessage(hCtl, EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
		SendMessage(hCtl, EM_GETSEL, 0, (LPARAM)&iEnd2);
		if (iEnd2 == iEnd) { return TRUE; }
	}
	return FALSE;
}

EXTERN_C HRESULT OSGetHwndText(HWND hwnd, LPTSTR *ppsz, int *pLen)
{
	HRESULT hr = 0;
	int nTxt = 0;
	LPTSTR psz = NULL;

	if (!IsWindow(hwnd) || (!ppsz && !pLen)) {
		hr = E_INVALIDARG; goto eof;
	}

	nTxt = GetWindowTextLength(hwnd);
	if (nTxt < 1) goto eof;
	if (pLen) { *pLen = nTxt; }

	if (ppsz)
	{
		int const nMax = nTxt + 1;
		psz = MemAllocZero(nMax * sizeof(TCHAR));
		if (!psz) {
			hr = E_OUTOFMEMORY; goto eof;
		}
		GetWindowText(hwnd, psz, nMax);
		*ppsz = psz;
	}
eof:
	if (FAILED(hr)) {
		APP_SafeFree(psz, MemFree);
	}
	return hr;
}

EXTERN_C HRESULT OSGetHwndTextW(HWND hwnd, LPWSTR *ppsz, int *pCount)
{
#ifndef UNICODE
	HRESULT hr = 0;
	LPSTR pszA = NULL;
	LPWSTR pszW = NULL;

	hr = OSGetHwndText(hwnd, &pszA, NULL);
	if (!pszA) goto eof;
	
	hr = OSAllocWideFromMulti(&pszW, CP_ACP, pszA, -1);
	if (FAILED(hr)) goto eof;

	if (pCount) {
		int i = 0; for (; pszW[i]; ++i);
		*pCount = i;
	}
	*ppsz = pszW;
eof:
	APP_SafeFree(pszA, MemFree);
	return hr;
#else
	return OSGetHwndText(hwnd, ppsz, pCount);
#endif
}

EXTERN_C int OSGetDPI(void)
{
	int dpi = 96;
	HDC hdc0 = GetDC(NULL);
	if (hdc0) {
		dpi = GetDeviceCaps(hdc0, LOGPIXELSY);
		ReleaseDC(NULL, hdc0);
	}
	return dpi;
}

EXTERN_C BYTE OSGetOSMajorVer(void)
{
	typedef DWORD(WINAPI *fn_t)(void);
	fn_t fn = (fn_t)GetProcAddress(LoadLibraryA("kernel32"),
		"GetVersion");
	if (fn) {
		DWORD ver = fn();
		return (BYTE)(ver & 0xFF);
	}
	return 0;
}

EXTERN_C WNDPROC OSGetButtonDefWndProc(void)
{
	WNDCLASS wc = { 0 };
	GetClassInfo(NULL, WC_BUTTON, &wc);
	return wc.lpfnWndProc;
}

EXTERN_C WNDPROC OSGetEditDefWndProc(void)
{
	WNDCLASS wc = { 0 };
	GetClassInfo(NULL, WC_EDIT, &wc);
	return wc.lpfnWndProc;
}

EXTERN_C BOOL OSQueryMessageFont(LOGFONT *pLF)
{
	BOOL ok = FALSE;
	NCM_t ncm = { sizeof(ncm) };
	ok = SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof(ncm), &ncm, 0);
	*pLF = ncm.lfMessageFont;
	return ok;
}

EXTERN_C HRESULT OSSetClipboardTextA(LPCSTR pszStr, SIZE_T cbStr)
{
	HRESULT hr = 0;
	BOOL ok = FALSE;
	HANDLE hMem = NULL;
	LPVOID pMem = NULL;
	
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, cbStr + 1);
	if (!hMem) goto eof;
	pMem = GlobalLock(hMem);
	if (!pMem) goto eof;
	CopyMemory(pMem, pszStr, cbStr);
	GlobalUnlock(hMem);
	ok = OpenClipboard(NULL)
		&& EmptyClipboard()
		&& SetClipboardData(CF_TEXT, hMem)
		&& CloseClipboard();
eof:
	if (!ok) {
		hr = APP_GetLastHr(); goto eof;
	}
	return hr;
}

EXTERN_C BOOL OSSupportGBK(void)
{
	HRESULT hr = 0;
	LPSTR psz = NULL;
	hr = OSAllocMultiFromWide(&psz, CP_GBK_2312, L"\x3042\x6E2C", -1);
	APP_SafeFree(psz, MemFree);
	return (hr == S_OK);
}
