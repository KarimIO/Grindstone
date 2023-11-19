#pragma once

#include <string>
#include <filesystem>
#include <Common/Graphics/WindowGraphicsBinding.hpp>
#include <Common/Display/Display.hpp>
#include <Common/Input/CursorMode.hpp>

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
			FullscreenMode fullscreen = FullscreenMode::Windowed;
			Grindstone::Display display;
			unsigned int width = 0;
			unsigned int height = 0;
			const char* title = nullptr;
			bool isSwapchainControlledByEngine = false;
		};
	public:
		static Grindstone::Window* Create(CreateInfo& createInfo);
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual bool ShouldClose() = 0;
		virtual void HandleEvents() = 0;
		virtual void SetFullscreen(FullscreenMode mode) = 0;
		virtual void GetWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) const = 0;
		virtual void GetWindowSize(unsigned int& width, unsigned int& height) const = 0;
		virtual void SetWindowSize(unsigned int width, unsigned int height) = 0;
		virtual void GetMousePos(unsigned int& x, unsigned int& y) const = 0;
		virtual void SetMousePos(unsigned int x, unsigned int y) = 0;
		virtual void SetCursorMode(Grindstone::Input::CursorMode cursorMode) = 0;
		virtual Grindstone::Input::CursorMode GetCursorMode() const = 0;
		virtual void SetMouseIsRawMotion(bool isRawMotion) = 0;
		virtual bool GetMouseIsRawMotion() const = 0;
		virtual void SetWindowPos(unsigned int x, unsigned int y) = 0;
		virtual void GetWindowPos(unsigned int& x, unsigned int& y) const = 0;
		virtual void SetWindowFocus(bool isFocused) = 0;
		virtual bool GetWindowFocus() const = 0;
		virtual bool GetWindowMinimized() const = 0;
		virtual void GetTitle(char* allocatedBuffer) const = 0;
		virtual void SetTitle(const char* title) = 0;
		virtual void SetWindowAlpha(float alpha) = 0;
		virtual float GetWindowDpiScale() const = 0;
		virtual void Close() = 0;

		virtual bool CopyStringToClipboard(const std::string& stringToCopy) = 0;
		virtual std::filesystem::path BrowseFolder(std::filesystem::path& defaultPath) = 0;
		virtual std::filesystem::path OpenFileDialogue(const char* filter = "All Files (*.*)\0*.*\0") = 0;
		virtual std::filesystem::path SaveFileDialogue(const char* filter = "All Files (*.*)\0*.*\0") = 0;
		virtual void ExplorePath(const char* path) = 0;
		virtual void OpenFileUsingDefaultProgram(const char* path) = 0;
	public:
		inline Grindstone::GraphicsAPI::WindowGraphicsBinding* GetWindowGraphicsBinding() const {
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

		inline bool IsSwapchainControlledByEngine() const {
			return isSwapchainControlledByEngine;
		}
	protected:
		Grindstone::GraphicsAPI::WindowGraphicsBinding* windowsGraphicsBinding = nullptr;
		bool isSwapchainControlledByEngine;
	private:
		virtual bool Initialize(CreateInfo& createInfo) = 0;
	};
};
