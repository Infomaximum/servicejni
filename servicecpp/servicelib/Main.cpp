#include <Windows.h>

#if defined(WINDOWS)
BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD /*ul_reason_for_call*/, LPVOID /*pReserved*/)
{
	return TRUE;
}
#endif

