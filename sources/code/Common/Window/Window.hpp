#pragma once

#include <Common/Graphics/WindowGraphicsBinding.hpp>
#include <Common/Display/Display.hpp>

class InputInterface;


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
			EngineCore* engine_core = nullptr;
			FullscreenMode fullscreen;
			Grindstone::Display display;
			unsigned int width;
			unsigned int height;
			const char* title;
		};
	public:
		static Window* create(CreateInfo& create_info);
		virtual void show() = 0;
		virtual bool shouldClose() = 0;
		virtual bool handleEvents() = 0;
		virtual void setFullscreen(FullscreenMode mode) = 0;
		virtual void getWindowRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom) = 0;
		virtual void getWindowSize(unsigned int& width, unsigned int& height) = 0;
		virtual void setWindowSize(unsigned int width, unsigned int height) = 0;
		virtual void getMousePos(unsigned int& x, unsigned int& y) = 0;
		virtual void setMousePos(unsigned int x, unsigned int y) = 0;
		virtual void setWindowPos(unsigned int x, unsigned int y) = 0;
		virtual void getWindowPos(unsigned int& x, unsigned int& y) = 0;
		virtual void setWindowFocus() = 0;
		virtual bool getWindowFocus() = 0;
		virtual bool getWindowMinimized() = 0;
		virtual void setWindowTitle(const char* title) = 0;
		virtual void setWindowAlpha(float alpha) = 0;
		virtual float getWindowDpiScale() = 0;
		virtual void close() = 0;
	public:
		inline Grindstone::GraphicsAPI::WindowGraphicsBinding* getWindowGraphicsBinding() {
			return windows_graphics_binding_;
		}

		inline void addBinding(Grindstone::GraphicsAPI::WindowGraphicsBinding* wgb) {
			windows_graphics_binding_ = wgb;
		}

		inline void immediateSetContext() {
			windows_graphics_binding_->immediateSetContext();
		}

		inline void immediateSwapBuffers() {
			windows_graphics_binding_->immediateSwapBuffers();
		}
	protected:
		Grindstone::GraphicsAPI::WindowGraphicsBinding* windows_graphics_binding_;
	private:
		virtual bool initialize(CreateInfo& create_info) = 0;
	};
};