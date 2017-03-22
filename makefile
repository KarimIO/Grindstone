# Compiler
CC=g++

# C Files
CFLAGS=-c

# Object Definitions
CPP_CORE_FILES := $(wildcard sources/code/Engine/Core/*.cpp)
OBJ_CORE_FILES := $(addprefix sources/obj/Engine/Core/,$(notdir $(CPP_CORE_FILES:.cpp=.o)))
CPP_ENTS_FILES := $(wildcard sources/code/Engine/Entities/*.cpp)
OBJ_ENTS_FILES := $(addprefix sources/obj/Engine/Entities/,$(notdir $(CPP_ENTS_FILES:.cpp=.o)))
CPP_RENDER_FILES := $(wildcard sources/code/Engine/Renderpaths/*.cpp)
OBJ_RENDER_FILES := $(addprefix sources/obj/Engine/Renderpaths/,$(notdir $(CPP_RENDER_FILES:.cpp=.o)))
CPP_SYSTEMS_FILES := $(wildcard sources/code/Engine/Systems/*.cpp)
OBJ_SYSTEMS_FILES := $(addprefix sources/obj/Engine/Systems/,$(notdir $(CPP_SYSTEMS_FILES:.cpp=.o)))

CPP_ENGINE_FILES := $(CPP_CORE_FILES) $(CPP_ENTS_FILES) $(CPP_RENDER_FILES) $(CPP_SYSTEMS_FILES)
OBJ_ENGINE_FILES := $(OBJ_CORE_FILES) $(OBJ_ENTS_FILES) $(OBJ_RENDER_FILES) $(OBJ_SYSTEMS_FILES)

INCLUDE_PATHS := -I sources/include/ -I sources/code/Engine/ -I sources/code/Engine/Core -I sources/code/Engine/Entities -I sources/code/Engine/Renderpaths -I sources/code/Engine/Systems -I sources/code/GraphicsOpenGL/ -I sources/code/WindowModule/  -I sources/code/GraphicsCommon/ -I sources/code/AudioModule/ -I sources/include/STB/ -I sources/include/rapidjson/include/

all: build

rebuild: clean build

build: Engine OpenGLModule WindowModule

# Engine code
sources/obj/Engine/Core/%.o: sources/code/Engine/Core/%.cpp
	@mkdir -p sources/obj/Engine/Core
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -fPIC -o $@ $< -ldl -lX11 -lassimp -lGL -std=c++11
	
sources/obj/Engine/Entities/%.o: sources/code/Engine/Entities/%.cpp
	@mkdir -p sources/obj/Engine/Entities
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -fPIC -o $@ $< -ldl -lX11 -lassimp -lGL -std=c++11
	
sources/obj/Engine/Renderpaths/%.o: sources/code/Engine/Renderpaths/%.cpp
	@mkdir -p sources/obj/Engine/Renderpaths
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -fPIC -o $@ $< -ldl -lX11 -lassimp -lGL -std=c++11
	
sources/obj/Engine/Systems/%.o: sources/code/Engine/Systems/%.cpp
	@mkdir -p sources/obj/Engine/Systems
	@$(CC) $(INCLUDE_PATHS) $(CFLAGS) -fPIC -o $@ $< -ldl -lX11 -lassimp -lGL -std=c++11
	
Engine: $(OBJ_ENGINE_FILES)
	@mkdir -p sources/obj/Engine
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
	@$(CC) -shared $^ -o bin/graphicsgl.so -lX11 -lGL
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
	@rm bin/graphicsgl.so
	@rm bin/window.so
