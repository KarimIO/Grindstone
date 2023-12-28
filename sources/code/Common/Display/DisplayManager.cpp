#include "DisplayManager.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>

static BOOL CALLBACK EnumDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam) {
	Display* pArg = reinterpret_cast<Display*>(lParam);

	// TODO: Fix all this
	Display &display = *pArg;
	display.monitorId = 0;
	display.x = pRcMon->left;
	display.y = pRcMon->top;
	display.width = pRcMon->right - pRcMon->left;
	display.height = pRcMon->bottom - pRcMon->top;

	return TRUE;
}

static BOOL CALLBACK CountDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam) {
	uint8_t& count = *reinterpret_cast<uint8_t*>(lParam);
	count++;
	return TRUE;
}

Display DisplayManager::GetMainDisplay() {
	Display* displays = new Display[GetDisplayCount()];
	EnumerateDisplays(displays);

	return displays[0];
}

uint8_t DisplayManager::GetDisplayCount() {
	uint8_t count = 0;
	EnumDisplayMonitors(0, 0, CountDispProc, reinterpret_cast<LPARAM>(&count));
	return count;
}

void DisplayManager::EnumerateDisplays(Display *displays) {
	EnumDisplayMonitors(0, 0, EnumDispProc, reinterpret_cast<LPARAM>(displays));
}
#elif defined(__linux__)
Display DisplayManager::GetMainDisplay() {
	return Display();
}

uint8_t DisplayManager::GetDisplayCount() {
	return 0;
}

void DisplayManager::EnumerateDisplays(Display *displays) {
}
#endif
