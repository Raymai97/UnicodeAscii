#pragma once
#include "AppEditBox.h"

typedef struct MySelf MySelf_t;
struct MySelf {
	HWND hwndSelf;
	HFONT hfoSelf;
	LOGFONT lfSelf;
	BOOL zoomEnabled;
	App_FontZoomSpec_t zoomSpec;
};


EXTERN_C BOOL AppEditBox_SetFontCopy(
	MySelf_t *pSelf, BOOL fRedraw,
	LOGFONT const *pLF
);

EXTERN_C BOOL AppEditBox_SetFontZoomSpec(
	MySelf_t *pSelf, BOOL enable,
	App_FontZoomSpec_t const *pSpec
);

/*
	zoom:
	  > 0 ? zoom in
	  < 0 ? zoom out
	  zero ? zoom reset
*/
EXTERN_C BOOL AppEditBox_Zoom(
	MySelf_t *pSelf, int zoom
);

