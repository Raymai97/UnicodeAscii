#include "AppEditBox.h"
#include "AppEditBox_internal.h"

#define MY_SafeFree			APP_SafeFree

static WNDPROC s_dwpEdit = NULL;


static LRESULT MyWndProc(
	MySelf_t *pSelf, UINT msg, WPARAM w, LPARAM l
);

static BOOL MyOnKeyDown(
	MySelf_t *pSelf, UINT vk
);

static BOOL MyOnMouseWheel(
	MySelf_t *pSelf, WORD mk, short delta
);

static void MySendCommandToParent(
	HWND hCtl, WORD nCode
);



EXTERN_C void AppEditBox_Init(void)
{
	s_dwpEdit = OSGetEditDefWndProc();
}


EXTERN_C BOOL AppEditBox_SetFontCopy(
	MySelf_t *pSelf, BOOL fRedraw,
	LOGFONT const *pLF)
{
	HWND const hCtl = pSelf->hwndSelf;
	HFONT hfoCopy = NULL;
	hfoCopy = CreateFontIndirect(pLF);
	if (hfoCopy != NULL) {
		SendMessage(hCtl, WM_SETFONT, (WPARAM)hfoCopy, fRedraw);
		MY_SafeFree(pSelf->hfoSelf, DeleteObject);
		pSelf->hfoSelf = hfoCopy;
		pSelf->lfSelf = *pLF;
	}
	return (hfoCopy != NULL);
}


EXTERN_C LRESULT CALLBACK AppEditBox_WndProc(
	HWND hCtl, UINT msg, WPARAM w, LPARAM l)
{
	LPCSTR const pszpropSelf = "SelfPtr";
	LRESULT lResult = 0;
	MySelf_t *pSelf = NULL;
	if (msg == WM_NCDESTROY)
	{
		pSelf = RemovePropA(hCtl, pszpropSelf);
	}
	else
	{
		pSelf = GetPropA(hCtl, pszpropSelf);
		if (!pSelf)
		{
			pSelf = MemAllocZero(sizeof(*pSelf));
			pSelf->hwndSelf = hCtl;
			SetPropA(hCtl, pszpropSelf, pSelf);
		}
	}
	if (pSelf)
	{
		lResult = MyWndProc(pSelf, msg, w, l);
	}
	if (msg == WM_NCDESTROY)
	{
		MY_SafeFree(pSelf, MemFree);
	}
	return lResult;
}


static LRESULT MyWndProc(
	MySelf_t *pSelf, UINT msg, WPARAM w, LPARAM l)
{
	HWND const hCtl = pSelf->hwndSelf;
	LRESULT lResult = 0;
	BOOL overriden = FALSE;
	if (msg == WM_NCDESTROY)
	{
		MY_SafeFree(pSelf->hfoSelf, DeleteObject);
	}
	else if (msg == WM_KEYDOWN)
	{
		overriden = MyOnKeyDown(pSelf, (UINT)w);
	}
	else if (msg == WM_MOUSEWHEEL)
	{
		overriden = MyOnMouseWheel(pSelf, LOWORD(w), HIWORD(w));
	}
	else if (msg == EM_SetFontCopy)
	{
		lResult = AppEditBox_SetFontCopy(pSelf, (BOOL)w,
			(LOGFONT const *)l);
		overriden = TRUE;
	}
	else if (msg == EM_SetFontZoomSpec)
	{
		lResult = AppEditBox_SetFontZoomSpec(pSelf, (BOOL)w,
			(EM_FontZoomSpec_t const *)l);
		overriden = TRUE;
	}
	if (!overriden)
	{
		lResult = CallWindowProc(s_dwpEdit, hCtl, msg, w, l);
	}
	if (msg == WM_GETDLGCODE)
	{
		UINT vk = (UINT)w;
		/* Delegate TAB key to parent window. */
		if (vk == VK_TAB) {
			lResult &= ~(DLGC_WANTALLKEYS);
		}
	}
	return lResult;
}

static BOOL MyOnKeyDown(
	MySelf_t *pSelf, UINT vk)
{
	HWND const hCtl = pSelf->hwndSelf;
	BOOL ctrl = GetAsyncKeyState(VK_CONTROL) < 0;
	/* Select All */
	if ((ctrl && vk == 'A') || (vk == VK_ESCAPE))
	{
		SendMessage(hCtl, EM_SETSEL, 0, (LPARAM)-1);
	}
	else if ((ctrl && vk == VK_OEM_PLUS) ||
		(ctrl && vk == VK_ADD))
	{
		AppEditBox_Zoom(pSelf, +1);
	}
	else if ((ctrl && vk == VK_OEM_MINUS) ||
		(ctrl && vk == VK_SUBTRACT))
	{
		AppEditBox_Zoom(pSelf, -1);
	}
	else if ((ctrl && vk == '0') ||
		(ctrl && vk == VK_NUMPAD0))
	{
		AppEditBox_Zoom(pSelf, 0);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL MyOnMouseWheel(
	MySelf_t *pSelf, WORD mk, short delta)
{
	BOOL overriden = FALSE;
	/* If CTRL is down when mouse-wheel rotated */
	if (mk & MK_CONTROL)
	{
		AppEditBox_Zoom(pSelf, delta);
		overriden = TRUE;
	}
	return overriden;
}

static void MySendCommandToParent(
	HWND hCtl, WORD nCode)
{
	SendMessage(GetParent(hCtl), WM_COMMAND,
		(nCode << 16) | GetDlgCtrlID(hCtl),
		(LPARAM)hCtl);
}

