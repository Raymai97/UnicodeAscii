#include "MainWin_internal.h"

static LPCSTR const s_pszSelfPtr = "SelfPtr";


static LRESULT CALLBACK MyWndProc(
	HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT lResult = 0;
	BOOL overriden = FALSE;
	MainWin_t *pSelf = NULL;
	/*
		Prepare pSelf
	*/
	if (msg == WM_NCCREATE)
	{
		pSelf = ((CREATESTRUCT*)l)->lpCreateParams;
		pSelf->hwndSelf = hwnd;
		SetPropA(hwnd, s_pszSelfPtr, pSelf);
	}
	else if (msg == WM_NCDESTROY)
	{
		pSelf = RemovePropA(hwnd, s_pszSelfPtr);
	}
	else
	{
		pSelf = GetPropA(hwnd, s_pszSelfPtr);
	}
	/*
		Pre DefWindowProc
	*/
	if (msg == WM_CREATE)
	{
		MainWin_OnLoad(pSelf);
	}
	else if (msg == WM_DESTROY)
	{
		MainWin_OnUnload(pSelf);
		PostQuitMessage(0);
	}
	else if (msg == WM_COMMAND)
	{
		MainWin_OnCommand(pSelf, LOWORD(w), HIWORD(w), (HWND)l);
	}
	else if (msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLORBTN)
	{
		/* Fix background color of Win3.1 and NT3.51 */
		HDC hdc = (HDC)w;
		SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
		lResult = (LRESULT)GetSysColorBrush(COLOR_3DFACE);
		overriden = TRUE;
	}
	if (!overriden)
	{
		lResult = DefWindowProc(hwnd, msg, w, l);
	}
	/*
		Post DefWindowProc
	*/
	if (msg == WM_ACTIVATE)
	{
		if (w == WA_INACTIVE) {
			pSelf->hwndFocus = GetFocus();
		}
		else {
			SetFocus(pSelf->hwndFocus);
		}
	}
	return lResult;
}


EXTERN_C HRESULT MainWin_ShowModal(void)
{
	HRESULT hr = 0;
	MainWin_t *pSelf = NULL;
	HWND hwnd = NULL;
	WNDCLASS wc = { 0 };
	MSG msg = { 0 };

	/*
		Do not init any UI stuff here.
		Init/Uninit UI stuff in WndProc only.
	*/
	pSelf = MemAllocZero(sizeof(*pSelf));
	if (!pSelf) {
		hr = E_OUTOFMEMORY; goto eof;
	}

	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = s_pszSelfClass;
	wc.lpfnWndProc = MyWndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);
	hwnd = CreateWindowEx(0, s_pszSelfClass, NULL,
		WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
		CW_USEDEFAULT, 0, 0, 0, NULL, NULL, NULL, pSelf);
	if (!hwnd) {
		hr = E_UNEXPECTED; goto eof;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
eof:
	if (pSelf) { MemFree(pSelf); }
	return hr;
}

