#include "AppEditBox_internal.h"

#define MY_SafeFree		APP_SafeFree


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
	if (!hfoCurr)  goto eof;
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
