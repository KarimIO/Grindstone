#pragma once

#define NOMINMAX

#include "Window.hpp"
#include <windows.h>

class InputInterface;

namespace Grindstone {
	class Win32Window : public Window {
	public:
		virtual bool initialize(CreateInfo create_info) override;
		virtual void show() override;
		virtual bool shouldClose() override;
		virtual bool handleEvents() override;
		virtual void setFullscreen(FullscreenMode mode) override;
		virtual void getWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) override;
		virtual void getWindowSize(unsigned int& width, unsigned int& height) override;
		virtual void setWindowSize(unsigned int width, unsigned int height) override;
		virtual void setMousePos(unsigned int x, unsigned int y) override;
		virtual void getMousePos(unsigned int& x, unsigned int& y) override;
		virtual void setWindowPos(unsigned int x, unsigned int y) override;
		virtual void getWindowPos(unsigned int& x, unsigned int& y) override;
		virtual void setWindowFocus() override;
		virtual bool getWindowFocus() override;
		virtual bool getWindowMinimized() override;
		virtual void setWindowTitle(const char* title) override;
		virtual void setWindowAlpha(float alpha) override;
		virtual float getWindowDpiScale() override;
		virtual void close() override;
	public:
		HWND getHandle();
	private:
		static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT	CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND	window_handle_;
		RECT	window_size_;
		unsigned int width_;
		unsigned int height_;
		FullscreenMode fullscreen_mode_;
		DWORD style_;
		DWORD ex_style_;
		InputInterface* input_;
		bool should_close_;
	};
};
