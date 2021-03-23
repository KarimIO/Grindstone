#pragma once

#define NOMINMAX

#include "Window.hpp"
#include <windows.h>
#include <Common/Input.hpp>

class InputInterface;

namespace Grindstone {
	class EngineCore;

	class Win32Window : public Window {
	public:
		virtual bool Initialize(CreateInfo& create_info) override;
		virtual void Show() override;
		virtual bool ShouldClose() override;
		virtual bool HandleEvents() override;
		virtual void SetFullscreen(FullscreenMode mode) override;
		virtual void GetWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) override;
		virtual void GetWindowSize(unsigned int& width, unsigned int& height) override;
		virtual void SetWindowSize(unsigned int width, unsigned int height) override;
		virtual void SetMousePos(unsigned int x, unsigned int y) override;
		virtual void GetMousePos(unsigned int& x, unsigned int& y) override;
		virtual void SetWindowPos(unsigned int x, unsigned int y) override;
		virtual void GetWindowPos(unsigned int& x, unsigned int& y) override;
		virtual void SetWindowFocus() override;
		virtual bool GetWindowFocus() override;
		virtual bool GetWindowMinimized() override;
		virtual void SetWindowTitle(const char* title) override;
		virtual void SetWindowAlpha(float alpha) override;
		virtual float GetWindowDpiScale() override;
		virtual void Close() override;
	public:
		HWND GetHandle();
	private:
		static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT	CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND	windowHandle;
		RECT	windowSize;
		unsigned int width;
		unsigned int height;
		FullscreenMode fullscreenMode;
		DWORD style;
		DWORD extendedStyle;
		EngineCore* engineCore = nullptr;
		bool shouldClose;
	};
};
