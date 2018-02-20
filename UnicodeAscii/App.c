#include "App.h"

#define MY_CodepageNotSupported(cp) \
	((cp) != CP_ACP && (cp) != CP_OEMCP && !IsValidCodePage(cp))

typedef struct App App_t;
struct App {
	int dpi;
	WNDPROC dwpEdit;
};

static App_t s_App;


static BOOL MyInitComCtl(void)
{
	typedef INITCOMMONCONTROLSEX ICCEX;
	typedef BOOL(WINAPI *fn_t)(ICCEX const *);
	fn_t fn = (fn_t)GetProcAddress(LoadLibraryA("comctl32"),
		"InitCommonControlsEx");
	if (fn) {
		ICCEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
		return fn(&icc);
	}
	return FALSE;
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
	MyInitComCtl();
	s_App.dpi = OSGetDPI();
	s_App.dwpEdit = OSGetEditDefWndProc();
}

static int CALLBACK MyFontEnumProc_CheckAvail(
	LOGFONT const *lplf, TEXTMETRIC const *lptm,
	DWORD dwType, LPARAM user)
{
	UNREFERENCED_PARAMETER(lplf);
	UNREFERENCED_PARAMETER(lptm);
	UNREFERENCED_PARAMETER(dwType);
	(*(BOOL*)user) = TRUE;
	return 0;
}

EXTERN_C HFONT App_CreateFont(
	LPCTSTR pszName, int ptSize)
{
	int const dpi = s_App.dpi;
	HFONT hfo = NULL;
	BOOL available = FALSE;
	HDC hdc0 = NULL;
	hdc0 = GetDC(NULL);
	if (!hdc0) goto eof;

	EnumFonts(hdc0, pszName, MyFontEnumProc_CheckAvail,
		(LPARAM)&available);
	if (!available) goto eof;

	hfo = CreateFont(
		-MulDiv(ptSize, dpi, 72),
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

EXTERN_C LRESULT CALLBACK App_CtlWndProc_Edit(
	HWND hCtl, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT lResult = 0;
	BOOL overriden = FALSE;
	if (msg == WM_KEYDOWN)
	{
		/* If ESC or CTRL+A, Select All. */
		if (GetAsyncKeyState(VK_ESCAPE) < 0 ||
			(GetAsyncKeyState(VK_CONTROL) < 0 &&
			GetAsyncKeyState('A') < 0))
		{
			SendMessage(hCtl, EM_SETSEL, 0, (LPARAM)-1);
			overriden = TRUE;
		}
	}
	if (!overriden)
	{
		lResult = CallWindowProc(s_App.dwpEdit, hCtl, msg, w, l);
	}
	if (msg == WM_GETDLGCODE)
	{
		/* Delegate TAB key to parent window. */
		if (w == VK_TAB) {
			lResult &= ~(DLGC_WANTALLKEYS);
		}
	}
	return lResult;
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

	hr = OSGetHwndText(hwnd, &pszA, pCount);
	if (!pszA) goto eof;
	
	hr = OSAllocWideFromMulti(&pszW, CP_ACP, pszA, -1);
	if (FAILED(hr)) goto eof;
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

EXTERN_C WNDPROC OSGetEditDefWndProc(void)
{
	WNDCLASS wc = { 0 };
	GetClassInfo(NULL, WC_EDIT, &wc);
	return wc.lpfnWndProc;
}

EXTERN_C BOOL OSGetUserUILang(LANGID *pID)
{
	typedef LANGID(WINAPI *fn_t)(void);
	fn_t fn = (fn_t)GetProcAddress(LoadLibraryA("kernel32"),
		"GetUserDefaultUILanguage");
	if (fn) {
		*pID = fn();
	}
	return (fn != NULL);
}

EXTERN_C BOOL OSGetUserLangID(LANGID *pID)
{
	typedef LANGID(WINAPI *fn_t)(void);
	fn_t fn = (fn_t)GetProcAddress(LoadLibraryA("kernel32"),
		"GetUserDefaultLangID");
	if (fn) {
		*pID = fn();
	}
	return (fn != NULL);
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

