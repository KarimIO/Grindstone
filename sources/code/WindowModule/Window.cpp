#include "Window.h"

WINDOW_EXPORT GameWindow* createWindow() {
  return new GameWindow;
}

WINDOW_EXPORT void destroyObject( GameWindow* object ) {
  delete object;
}