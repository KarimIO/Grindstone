#pragma once

#include "Window.hpp"
#include <X11/Xlib.h>

class InputInterface;

namespace Grindstone {
	class X11Window : public Grindstone::Window {
	public:
		virtual bool Initialize(CreateInfo& createInfo) override;
		virtual void Show() override;
		virtual bool ShouldClose() override;
		virtual void HandleEvents() override;
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

		virtual bool CopyStringToClipboard(const std::string& stringToCopy) override;
		virtual std::string OpenFileDialogue(const char* filter) override;
		virtual std::string SaveFileDialogue(const char* filter) override;
		virtual void ExplorePath(const char* path) override;
		virtual void OpenFileUsingDefaultProgram(const char* path) override;
	public:
		::Window GetHandle();

	private:
		::Display* xDisplay;
		::Window xWindow;
		unsigned int width;
		unsigned int height;
		FullscreenMode fullscreenMode;
		EngineCore* engineCore = nullptr;
		bool shouldClose;
	};
};
