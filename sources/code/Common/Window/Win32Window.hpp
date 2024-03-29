#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Window.hpp"
#include <windows.h>
#include <Common/Input/InputInterface.hpp>

namespace Grindstone {
	class EngineCore;

	class Win32Window : public Window {
	public:
		virtual bool Initialize(CreateInfo& createInfo) override;
		virtual void Show() override;
		virtual void Hide() override;
		virtual bool ShouldClose() override;
		virtual void HandleEvents() override;
		virtual void SetFullscreen(FullscreenMode mode) override;
		virtual void GetWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) const override;
		virtual void GetWindowSize(unsigned int& width, unsigned int& height) const override;
		virtual void SetWindowSize(unsigned int width, unsigned int height) override;
		virtual void SetMousePos(unsigned int x, unsigned int y) override;
		virtual void GetMousePos(unsigned int& x, unsigned int& y) const override;
		virtual void SetWindowPos(unsigned int x, unsigned int y) override;
		virtual void GetWindowPos(unsigned int& x, unsigned int& y) const override;
		virtual bool GetWindowFocus() const override;
		virtual void SetWindowFocus(bool isFocused) override;
		virtual bool GetWindowMinimized() const override;
		virtual void GetTitle(char* allocatedBuffer) const override;
		virtual void SetTitle(const char* title) override;
		virtual void SetWindowAlpha(float alpha) override;
		virtual float GetWindowDpiScale() const override;
		virtual void Close() override;

		virtual bool CopyStringToClipboard(const std::string& stringToCopy) override;
		virtual std::filesystem::path BrowseFolder(std::filesystem::path& defaultPath) override;
		virtual std::filesystem::path OpenFileDialogue(const char* filter) override;
		virtual std::filesystem::path SaveFileDialogue(const char* filter) override;
		virtual void ExplorePath(const char* path) override;
		virtual void OpenFileUsingDefaultProgram(const char* path) override;
	public:
		HWND GetHandle() const;
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
