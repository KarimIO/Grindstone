#pragma once

class InputInterface;

enum class WindowFullscreenMode {
	Windowed = 0,
	Borderless,
	Fullscreen
};

struct WindowCreateInfo {
	InputInterface* input_interface;
	WindowFullscreenMode fullscreen;
	unsigned int width;
	unsigned int height;
	const char* title;
};

class BaseWindow {
public:
	virtual bool initialize(WindowCreateInfo create_info) = 0;
	virtual void show() = 0;
	virtual void setInputInterface(InputInterface* ii) = 0;
	virtual bool shouldClose() = 0;
	virtual bool handleEvents() = 0;
	virtual void setFullscreen(WindowFullscreenMode mode) = 0;
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
};
