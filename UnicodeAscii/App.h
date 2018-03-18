#pragma once
#include <Windows.h>
#include <CommCtrl.h>


/*
	DEFINEs
*/

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x)	(x)
#endif

#ifndef SetWindowLongPtr
#define SetWindowLongPtr	SetWindowLong
#endif

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC		GWL_WNDPROC
#endif

#ifndef WC_BUTTON
#define WC_BUTTON	TEXT("Button")
#endif

#ifndef WC_STATIC
#define WC_STATIC	TEXT("Static")
#endif

#ifndef WC_EDIT
#define WC_EDIT		TEXT("Edit")
#endif

#ifndef WC_COMBOBOX
#define WC_COMBOBOX		TEXT("ComboBox")
#endif

/*
	App DEFINEs
*/

#ifndef NDEBUG
#define WS_DEBUGBOX		(WS_BORDER)
#else
#define WS_DEBUGBOX		(0)
#endif

#define APP_Name		"UnicodeAscii"
#define APP_VerStr		"v1.1a"

#define APP_GetLastHr() \
	HRESULT_FROM_WIN32(GetLastError())

#define APP_SafeFree(p, fn) \
	if (p) { fn(p); p = NULL; }


/* ========== HRESULT extend for OSAlloc-Multi-Wide =========== */
/*
	Not all characters are converted.
	pszDst truncated until the unrepresentable char.
*/
#define	S_UNREPRESENTABLE_CHAR	\
	MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0)
#define E_CODEPAGE_NOT_SUPPORTED \
	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0)

/*
	App Struct .....
*/

typedef struct App_FontInfo {
	LPCSTR pszName;
	int ptSize;
} App_FontInfo_t;


/*
	App Global Constant ........
*/

enum App_Codepage {
	/*
		US English default.
	*/
	CP_Latin1 = 1252,
	/*
		Taiwan Traditional Chinese default.
	*/
	CP_Big5 = 950,
	/*
		China Simplified Chinese default.
		Represent GBK (or GB1232 if pre-Win2k)
	*/
	CP_GBK_2312 = 936,
	/*
		Japan Japanese default.
	*/
	CP_ShiftJIS = 932
};



/*
	App Global Memory Functions .........
*/

EXTERN_C LPVOID MemAllocZero(SIZE_T cb);
EXTERN_C void MemFree(LPVOID ptr);


/*
	App Specialized Shared Functions .........
*/

EXTERN_C void App_Init(void);

EXTERN_C HFONT App_CreateFont(
	App_FontInfo_t const *pInfo
);

EXTERN_C BOOL App_CreateEditBoxLogFont(LOGFONT *pLF);

EXTERN_C HFONT App_CreateEnglishFont(void);

EXTERN_C HWND App_CreateChild(
	HWND hwndParent, int id, LPCTSTR pszClass,
	LPCTSTR pszText, int x, int y, int cx, int cy,
	DWORD dwStyle, DWORD dwExStyle
);

EXTERN_C int App_FontSize_PtFromLog(int logSize);

EXTERN_C int App_FontSize_LogFromPt(int ptSize);

EXTERN_C void App_SetFocus(HWND hCtl);

EXTERN_C void App_SetWindowClientSize(
	HWND hwnd, int cx, int cy
);


/*
	Windows Helper Functions ........
*/

EXTERN_C HRESULT OSAllocMultiFromWide(
	LPSTR *ppszDst, UINT cpDst,
	LPCWSTR pszSrc, int cchSrc
);

EXTERN_C HRESULT OSAllocWideFromMulti(
	LPWSTR *ppszDst, UINT cpSrc,
	LPCSTR pszSrc, int cbSrc
);

EXTERN_C HRESULT OSAllocTStrFromMulti(
	LPTSTR *ppszDst, UINT cpSrc,
	LPCSTR pszSrc, int cbSrc
);

EXTERN_C BOOL OSEdit_SelectNextChar(HWND hCtl, int iStart);

/* This function set *ppsz only if no error and text length > 0. */
EXTERN_C HRESULT OSGetHwndText(HWND hwnd, LPTSTR *ppsz, int *pLen);

EXTERN_C HRESULT OSGetHwndTextW(HWND hwnd, LPWSTR *ppsz, int *pcch);

EXTERN_C int OSGetDPI(void);

EXTERN_C BOOL OSGetFontLogHeight(HFONT hfo, LONG *pLH);

EXTERN_C BYTE OSGetOSMajorVer(void);

EXTERN_C WNDPROC OSGetButtonDefWndProc(void);

EXTERN_C WNDPROC OSGetEditDefWndProc(void);

EXTERN_C BOOL OSQueryMessageFont(LOGFONT *pLF);

EXTERN_C HRESULT OSSetClipboardTextA(LPCSTR pszStr, SIZE_T cbStr);

EXTERN_C BOOL OSSupportGBK(void);
