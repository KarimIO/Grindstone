#pragma once

#include <string>
#include <filesystem>
#include <Common/Graphics/WindowGraphicsBinding.hpp>
#include <Common/Display/Display.hpp>

namespace Grindstone {
	class EngineCore;

	class Window {
	public:
		enum class FullscreenMode {
			Windowed = 0,
			Borderless,
			Fullscreen
		};

		struct CreateInfo {
			EngineCore* engineCore = nullptr;
			FullscreenMode fullscreen;
			Grindstone::Display display;
			unsigned int width;
			unsigned int height;
			const char* title;
		};
	public:
		static Grindstone::Window* Create(CreateInfo& createInfo);
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual bool ShouldClose() = 0;
		virtual void HandleEvents() = 0;
		virtual void SetFullscreen(FullscreenMode mode) = 0;
		virtual void GetWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) = 0;
		virtual void GetWindowSize(unsigned int& width, unsigned int& height) = 0;
		virtual void SetWindowSize(unsigned int width, unsigned int height) = 0;
		virtual void GetMousePos(unsigned int& x, unsigned int& y) = 0;
		virtual void SetMousePos(unsigned int x, unsigned int y) = 0;
		virtual void SetCursorIsVisible(bool isVisible) = 0;
		virtual bool GetCursorIsVisible() = 0;
		virtual void SetWindowPos(unsigned int x, unsigned int y) = 0;
		virtual void GetWindowPos(unsigned int& x, unsigned int& y) = 0;
		virtual void SetWindowFocus(bool isFocused) = 0;
		virtual bool GetWindowFocus() = 0;
		virtual bool GetWindowMinimized() = 0;
		virtual void GetTitle(char* allocatedBuffer) = 0;
		virtual void SetTitle(const char* title) = 0;
		virtual void SetWindowAlpha(float alpha) = 0;
		virtual float GetWindowDpiScale() = 0;
		virtual void Close() = 0;

		virtual bool CopyStringToClipboard(const std::string& stringToCopy) = 0;
		virtual std::filesystem::path BrowseFolder(std::filesystem::path& defaultPath) = 0;
		virtual std::filesystem::path OpenFileDialogue(const char* filter = "All Files (*.*)\0*.*\0") = 0;
		virtual std::filesystem::path SaveFileDialogue(const char* filter = "All Files (*.*)\0*.*\0") = 0;
		virtual void ExplorePath(const char* path) = 0;
		virtual void OpenFileUsingDefaultProgram(const char* path) = 0;
	public:
		inline Grindstone::GraphicsAPI::WindowGraphicsBinding* GetWindowGraphicsBinding() {
			return windowsGraphicsBinding;
		}

		inline void AddBinding(Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb) {
			windowsGraphicsBinding = wgb;
		}

		inline void ImmediateSetContext() {
			windowsGraphicsBinding->ImmediateSetContext();
		}

		inline void ImmediateSwapBuffers() {
			windowsGraphicsBinding->ImmediateSwapBuffers();
		}
	protected:
		Grindstone::GraphicsAPI::WindowGraphicsBinding* windowsGraphicsBinding;
	private:
		virtual bool Initialize(CreateInfo& createInfo) = 0;
	};
};
