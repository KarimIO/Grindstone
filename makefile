# Compiler
CC=g++

# C Files
CFLAGS=-c

# Object Definitions
CPP_ENGINE_FILES := $(wildcard sources/code/Engine/*.cpp)
OBJ_ENGINE_FILES := $(addprefix sources/obj/Engine/,$(notdir $(CPP_ENGINE_FILES:.cpp=.o)))

INCLUDE_PATHS := -I sources/code/Engine/ -I sources/code/GraphicsOpenGL/ -I sources/code/WindowModule/

all: build

rebuild: clean build

build: Engine OpenGLModule WindowModule

# Engine code
sources/obj/Engine/%.o: sources/code/Engine/%.cpp
	@mkdir -p sources/obj/Engine
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -o $@ $< -lX11 -lGL
	
Engine: $(OBJ_ENGINE_FILES)
	@$(CC) $^ -ldl -o bin/Grindstone
	@echo "Grindstone successfully built."

# Object Definitions
CPP_OPENGL_FILES := $(wildcard sources/code/GraphicsOpenGL/*.cpp)
OBJ_OPENGL_FILES := $(addprefix sources/obj/GraphicsOpenGL/,$(notdir $(CPP_OPENGL_FILES:.cpp=.o)))

# GraphicsModule code
Graphics: OpenGLModule
opengl: OpenGLModule

sources/obj/GraphicsOpenGL/%.o: sources/code/GraphicsOpenGL/%.cpp
	@mkdir -p sources/obj/GraphicsOpenGL
	@$(CC) -c -fPIC $< -o $@ -lX11 -lGL

OpenGLModule: $(OBJ_GRAPHICS_FILES)
	@$(CC) -shared $^ -o bin/opengl.so -lX11
	@echo "opengl.so successfully built."

# Object Definitions
CPP_WINDOW_FILES := $(wildcard sources/code/WindowModule/*.cpp)
OBJ_WINDOW_FILES := $(addprefix sources/obj/WindowModule/,$(notdir $(CPP_WINDOW_FILES:.cpp=.o)))

INCLUDE_WINDOW_PATHS := -I sources/code/Engine/

# WindowModule code
Window: WindowModule

sources/obj/WindowModule/%.o: sources/code/WindowModule/%.cpp
	@mkdir -p sources/obj/WindowModule
	@$(CC) $(INCLUDE_WINDOW_PATHS) -c -fPIC $< -o $@ -lX11 -lGL

WindowModule: $(OBJ_WINDOW_FILES)
	@$(CC) -shared $^ -o bin/window.so -lX11
	@echo "window.so successfully built."

clean:
	@rm -rf sources/obj/
	@rm bin/Grindstone
	@rm bin/opengl.so
	@rm bin/window.so
