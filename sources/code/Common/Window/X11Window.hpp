#pragma once

#include "Window.hpp"
#include <X11/Xlib.h>

class InputInterface;

namespace Grindstone {
	class X11Window : public Grindstone::Window {
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
		XID getHandle();

	private:
		XID w;
		unsigned int width_;
		unsigned int height_;
		FullscreenMode fullscreen_mode_;
		InputInterface* input_;
		bool should_close_;
	};
};
