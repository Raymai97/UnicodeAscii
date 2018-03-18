#include "AppEditBox_internal.h"

#define MY_SafeFree		APP_SafeFree


EXTERN_C BOOL AppEditBox_SetFontZoomSpec(
	MySelf_t *pSelf, BOOL enable,
	EM_FontZoomSpec_t const *pSpec)
{
	BOOL ok = FALSE;
	LONG logHeight = 0;
	pSelf->zoomEnabled = enable;
	if (!enable) goto eof;
	if (!pSelf->hfoSelf) goto eof;
	if (!pSpec) goto eof;
	if (pSpec->maxFontSize < pSpec->minFontSize) goto eof;
	if (pSpec->minFontSize > pSpec->maxFontSize) goto eof;
	if (pSpec->defFontSize) {
		if (pSpec->defFontSize < pSpec->minFontSize) goto eof;
		if (pSpec->defFontSize > pSpec->maxFontSize) goto eof;
		pSelf->defFontSize = pSpec->defFontSize;
	}
	else {
		ok = OSGetFontLogHeight(pSelf->hfoSelf, &logHeight);
		if (!ok) goto eof;
		pSelf->defFontSize = App_FontSize_PtFromLog(logHeight);
	}
	pSelf->minFontSize = pSpec->minFontSize;
	pSelf->maxFontSize = pSpec->maxFontSize;
	ok = AppEditBox_Zoom(pSelf, 0);
eof:
	if (!ok) {
		pSelf->zoomEnabled = FALSE;
	}
	return ok;
}

EXTERN_C BOOL AppEditBox_Zoom(
	MySelf_t *pSelf, int zoom)
{
	HFONT const hfoCurr = pSelf->hfoSelf;
	LOGFONT const *pLF = &pSelf->lfSelf;
	int const ptDef = pSelf->defFontSize;
	int const ptMin = pSelf->minFontSize;
	int const ptMax = pSelf->maxFontSize;
	BOOL ok = FALSE;
	int ptCurr = 0;
	int ptNew = 0;

	if (!pSelf->zoomEnabled) goto eof;
	if (!hfoCurr) goto eof;
	ok = TRUE;
	ptCurr = App_FontSize_PtFromLog(-(pLF->lfHeight));
	if (zoom > 0) /* zoom in */
	{
		int pt = ptCurr;
		pt += (pt < 8) ? (8 - pt) :
			(pt < 12) ? 1 :
			(pt < 24) ? 2 :
			(pt < 36) ? 4 : 12;
		ptNew = (pt > ptMax) ? ptMax : pt;
	}
	else if (zoom < 0) /* zoom out */
	{
		int pt = ptCurr;
		pt -= (pt > 36) ? 12 :
			(pt > 24) ? 4 :
			(pt > 12) ? 2 : 1;
		ptNew = (pt < ptMin) ? ptMin : pt;
	}
	else /* zoom reset */
	{
		ptNew = ptDef;
	}
	if (ptCurr != ptNew)
	{
		LOGFONT lf = *pLF;
		lf.lfHeight = -App_FontSize_LogFromPt(ptNew);
		ok = AppEditBox_SetFontCopy(pSelf, TRUE, &lf);
	}
eof:
	return ok;
}

