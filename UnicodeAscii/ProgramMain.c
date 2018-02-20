#include "App.h"
#include "MainWin.h"

void APIENTRY RawMain(void)
{
	App_Init();
	MainWin_ShowModal();
	ExitProcess(0);
}
