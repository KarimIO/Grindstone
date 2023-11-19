#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Window.hpp"
#include <windows.h>
#include <Common/Input/InputInterface.hpp>

struct GLFWwindow;

namespace Grindstone {
	class EngineCore;

	class GlfwWindow : public Window {
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
		virtual void SetCursorMode(Grindstone::Input::CursorMode cursorMode) override;
		virtual Grindstone::Input::CursorMode GetCursorMode() const override;
		virtual void SetMouseIsRawMotion(bool isRawMotion) override;
		virtual bool GetMouseIsRawMotion() const override;
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
		virtual GLFWwindow* GetHandle() const;
		EngineCore* engineCore = nullptr;
	private:
		GLFWwindow*	windowHandle;

		/*
		RECT	windowSize;
		unsigned int width;
		unsigned int height;
		FullscreenMode fullscreenMode;
		DWORD style;
		DWORD extendedStyle;
		bool shouldClose;
		*/
	};
};
