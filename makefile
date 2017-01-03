# Compiler
CC=g++

# C Files
CFLAGS=-c

# Object Definitions
CPP_ENGINE_FILES := $(wildcard sources/code/Engine/*.cpp)
OBJ_ENGINE_FILES := $(addprefix sources/obj/Engine/,$(notdir $(CPP_ENGINE_FILES:.cpp=.o)))

INCLUDE_PATHS := -I sources/include/ -I sources/code/Engine/ -I sources/code/GraphicsOpenGL/ -I sources/code/WindowModule/  -I sources/code/GraphicsCommon/

all: build

rebuild: clean build

build: Engine OpenGLModule WindowModule

# Engine code
sources/obj/Engine/%.o: sources/code/Engine/%.cpp
	@mkdir -p sources/obj/Engine
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -fPIC -o $@ $< -ldl -lX11 -lassimp -lGL -std=c++11
	
Engine: $(OBJ_ENGINE_FILES)
	@$(CC) $^ -ldl -lassimp -o bin/Grindstone
	@echo "Grindstone successfully built."

# Object Definitions
CPP_OPENGL_FILES := $(wildcard sources/code/GraphicsOpenGL/*.cpp)
OBJ_OPENGL_FILES := $(addprefix sources/obj/GraphicsOpenGL/,$(notdir $(CPP_OPENGL_FILES:.cpp=.o)))

INCLUDE_GRAPHICS_PATHS := -I sources/code/Engine/

# GraphicsModule code
Graphics: OpenGLModule
opengl: OpenGLModule

sources/obj/GraphicsOpenGL/%.o: sources/code/GraphicsOpenGL/%.cpp
	@mkdir -p sources/obj/GraphicsOpenGL
	@$(CC) $(INCLUDE_GRAPHICS_PATHS) -std=c++11 -c -fPIC $< -o $@ -lX11 -lGL

OpenGLModule: $(OBJ_OPENGL_FILES)
	@$(CC) -shared $^ -o bin/opengl.so -lX11 -lGL
	@echo "opengl.so successfully built."

# Object Definitions
CPP_WINDOW_FILES := $(wildcard sources/code/WindowModule/*.cpp)
OBJ_WINDOW_FILES := $(addprefix sources/obj/WindowModule/,$(notdir $(CPP_WINDOW_FILES:.cpp=.o)))

INCLUDE_WINDOW_PATHS := -I sources/code/Engine/ -I sources/code/GraphicsCommon/

# WindowModule code
Window: WindowModule

sources/obj/WindowModule/%.o: sources/code/WindowModule/%.cpp
	@mkdir -p sources/obj/WindowModule
	@$(CC) $(INCLUDE_WINDOW_PATHS) -std=c++11 -c -fPIC $< -o $@ -lX11 -lGL

WindowModule: $(OBJ_WINDOW_FILES)
	@$(CC) -shared $^ -o bin/window.so -lX11 -lGL
	@echo "window.so successfully built."

clean:
	@rm -rf sources/obj/
	@rm bin/Grindstone
	@rm bin/opengl.so
	@rm bin/window.so
