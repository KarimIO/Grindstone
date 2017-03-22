#include "Window.h"

void GameWindow::SetInputPointer(InputInterface *inputInterface) {
	input = inputInterface;
}

WINDOW_EXPORT GameWindow* createWindow() {
	return new GameWindow;
}