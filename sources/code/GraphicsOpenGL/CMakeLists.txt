set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(GL_CORE_SOURCES gl3w.cpp GLGraphicsGeneral.cpp GLGraphicsWindows.cpp GLGraphicsX11.cpp)
set(GL_CORE_HEADERS gl3w.h glcorearb.h OGLGraphicsWrapper.h)
set(GL_OBJ_SOURCES GLFramebuffer.cpp GLShader.cpp GLTexture.cpp GLVertexArrayObject.cpp GLVertexBufferObject.cpp GLUniformBuffer.cpp)
set(GL_OBJ_HEADERS GLFramebuffer.h GLShader.h GLTexture.h GLVertexArrayObject.h GLVertexBufferObject.h GLUniformBuffer.h)
set(GL_COMMON_HEADERS ../GraphicsCommon/Framebuffer.h ../GraphicsCommon/GLDefDLL.h ../GraphicsCommon/GraphicsWrapper.h ../GraphicsCommon/Shader.h ../GraphicsCommon/Texture.h ../GraphicsCommon/VertexArrayObject.h ../GraphicsCommon/VertexBufferObject.h ../GraphicsCommon/UniformBuffer.h)

set(GL_SOURCES ${GL_CORE_SOURCES} ${GL_OBJ_SOURCES})
set(GL_HEADERS ${GL_CORE_HEADERS} ${GL_OBJ_HEADERS} ${GL_COMMON_HEADERS})

source_group("Header Files\\Common Objects" FILES ${GL_COMMON_HEADERS})

source_group("Source Files\\Objects" FILES ${ENGINE_CORE_SOURCES})
source_group("Header Files\\Objects" FILES ${ENGINE_CORE_SOURCES})

add_library(graphicsgl MODULE ${GL_SOURCES} ${GL_HEADERS})

SET(GL3W_DIR "D:/Libraries/GL3W" CACHE PATH "Location of GL3W Library")
include_directories(${GL3W_DIR})
include_directories(${GLM_DIR})

target_compile_definitions(graphicsgl PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsgl opengl32)