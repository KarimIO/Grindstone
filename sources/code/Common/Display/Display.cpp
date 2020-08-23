#include "Display.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>

BOOL CALLBACK EnumDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam) {
	Display* pArg = reinterpret_cast<Display*>(lParam);

	// TODO: Fix all this
	Display &display = *pArg;
	display.monitor_id_ = 0;
	display.x_ = pRcMon->left;
	display.y_ = pRcMon->top;
	display.width_ = pRcMon->right - pRcMon->left;
	display.height_ = pRcMon->bottom - pRcMon->top;

	return TRUE;
}

BOOL CALLBACK CountDispProc(HMONITOR hMon, HDC dcMon, RECT* pRcMon, LPARAM lParam) {
	uint8_t* count = (uint8_t*)lParam;
	(*count)++;
	return TRUE;
}

Display Displays::getMainDisplay() {
	return Display();
}

uint8_t Displays::getDisplayCount() {
	uint8_t count = 0;
	EnumDisplayMonitors(0, 0, CountDispProc, reinterpret_cast<LPARAM>(&count));
	return count;
}

void Displays::enumerateDisplays(Display *displays) {
	EnumDisplayMonitors(0, 0, EnumDispProc, reinterpret_cast<LPARAM>(displays));
}
#elif defined(__linux__)
Display Displays::getMainDisplay() {
	return Display();
}

uint8_t Displays::getDisplayCount() {
	return 0;
}

void Displays::enumerateDisplays(Display *displays) {
}

#endif