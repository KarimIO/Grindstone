# Compiler
CC=g++

# C Files
CFLAGS=-c

# Object Definitions
CPP_ENGINE_FILES := $(wildcard sources/code/Engine/*.cpp)
OBJ_ENGINE_FILES := $(addprefix sources/obj/Engine/,$(notdir $(CPP_ENGINE_FILES:.cpp=.o)))

INCLUDE_PATHS := -I sources/code/GraphicsModule/ -I sources/code/WindowModule/

all: build

rebuild: clean build

build: Engine GraphicsModule WindowModule

# Engine code
sources/obj/Engine/%.o: sources/code/Engine/%.cpp
	@mkdir -p sources/obj/Engine
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -o $@ $< -lX11 -lGL
	
Engine: $(OBJ_ENGINE_FILES)
	@$(CC) $^ -ldl -o bin/Grindstone
	@echo "Grindstone successfully built."

# Object Definitions
CPP_GRAPHICS_FILES := $(wildcard sources/code/GraphicsModule/*.cpp)
OBJ_GRAPHICS_FILES := $(addprefix sources/obj/GraphicsModule/,$(notdir $(CPP_GRAPHICS_FILES:.cpp=.o)))

# GraphicsModule code
Graphics: GraphicsModule

sources/obj/GraphicsModule/%.o: sources/code/GraphicsModule/%.cpp
	@mkdir -p sources/obj/GraphicsModule
	@$(CC) -c -fPIC $< -o $@ -lX11 -lGL

GraphicsModule: $(OBJ_GRAPHICS_FILES)
	@$(CC) -shared $^ -o bin/graphics.so -lX11
	@echo "graphics.so successfully built."

# Object Definitions
CPP_WINDOW_FILES := $(wildcard sources/code/WindowModule/*.cpp)
OBJ_WINDOW_FILES := $(addprefix sources/obj/WindowModule/,$(notdir $(CPP_WINDOW_FILES:.cpp=.o)))

# WindowModule code
Window: WindowModule

sources/obj/WindowModule/%.o: sources/code/WindowModule/%.cpp
	@mkdir -p sources/obj/WindowModule
	@$(CC) -c -fPIC $< -o $@ -lX11 -lGL

WindowModule: $(OBJ_WINDOW_FILES)
	@$(CC) -shared $^ -o bin/window.so -lX11
	@echo "window.so successfully built."

clean:
	@rm -rf sources/obj/
	@rm bin/Grindstone
	@rm bin/graphics.so
	@rm bin/window.so
