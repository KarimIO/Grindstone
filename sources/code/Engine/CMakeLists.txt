set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENGINE_CORE_HEADERS Core/Engine.h Core/EBase.h Core/GraphicsDLLPointer.h Core/iniHandler.h Core/Input.h Core/InputInterface.h Core/TextureManager.h  Core/Space.h Core/Utilities.h Core/LevelLoader.h)
source_group("Core\\Header Files" FILES ${ENGINE_CORE_HEADERS})
set(ENGINE_RENDERPATHS_HEADERS Renderpaths/RenderPath.h  Renderpaths/RenderPathDeferred.h Renderpaths/RenderPathForward.h)
source_group("Render Paths\\Header Files" FILES ${ENGINE_RENDERPATHS_HEADERS})
set(ENGINE_SYSTEMS_HEADERS Systems/CController.h Systems/CTransform.h Systems/CAudio.h Systems/CBase.h Systems/CCamera.h Systems/CLight.h Systems/Physics.h Systems/SShader.h Systems/SAudio.h Systems/SCubemap.h Systems/SSkeletal.h Systems/SGeometry.h Systems/SLight.h Systems/Terrain.h)
source_group("Systems\\Header Files" FILES ${ENGINE_SYSTEMS_HEADERS})
set(ENGINE_POST_HEADERS PostProcess/BasePost.h PostProcess/DOFPost.h PostProcess/GaussianBlurPost.h PostProcess/PostPipeline.h PostProcess/SSAOPost.h PostProcess/ColorGradingPost.h)
source_group("Post Processing\\Header Files" FILES ${ENGINE_POST_HEADERS})
set(ENGINE_HEADERS ${ENGINE_CORE_HEADERS} ${ENGINE_ENTITES_HEADERS} ${ENGINE_RENDERPATHS_HEADERS} ${ENGINE_SYSTEMS_HEADERS} ${ENGINE_POST_HEADERS})

set(ENGINE_CORE_SOURCES Core/Main.cpp Core/EBase.cpp Core/Engine.cpp Core/iniHandler.cpp Core/Input.cpp Core/TextureManager.cpp Core/Space.cpp Core/Utilities.cpp Core/LevelLoader.cpp)
source_group("Core\\Source Files" FILES ${ENGINE_CORE_SOURCES})
set(ENGINE_RENDERPATHS_SOURCES Renderpaths/RenderPathDeferred.cpp Renderpaths/RenderPathForward.cpp)
source_group("Render Paths\\Source Files" FILES ${ENGINE_RENDERPATHS_SOURCES})
set(ENGINE_SYSTEMS_SOURCES Systems/CController.cpp Systems/CTransform.cpp Systems/CAudio.cpp Systems/CCamera.cpp Systems/CLight.cpp Systems/Physics.cpp Systems/SShader.cpp Systems/SAudio.cpp Systems/SCubemap.cpp Systems/SSkeletal.cpp Systems/SGeometry.cpp Systems/SLight.cpp Systems/Terrain.cpp)
source_group("Systems\\Source Files" FILES ${ENGINE_SYSTEMS_SOURCES})
set(ENGINE_POST_SOURCES PostProcess/DOFPost.cpp PostProcess/GaussianBlurPost.cpp PostProcess/PostPipeline.cpp  PostProcess/SSAOPost.cpp PostProcess/ColorGradingPost.cpp)
source_group("Post Processing\\Source Files" FILES ${ENGINE_POST_SOURCES})
set(ENGINE_SOURCES ${ENGINE_CORE_SOURCES} ${ENGINE_ENTITES_SOURCES} ${ENGINE_RENDERPATHS_SOURCES} ${ENGINE_SYSTEMS_SOURCES} ${ENGINE_POST_SOURCES}) 

set(INCLUDE_DIRECTORIES ../GraphicsCommon ../AudioModule)

add_executable(Grindstone ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_compile_definitions(Grindstone PRIVATE _CRT_SECURE_NO_WARNINGS)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set_target_properties(Grindstone PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
link_directories(${BULLET_ROOT_DIR}/${BULLET_LIBRARY_DIRS})

if (WIN32)
    target_link_libraries(Grindstone debug ${BULLET_DEBUG_LIBS})

    target_link_libraries(Grindstone optimized ${BULLET_RELEASE_LIBS})
else ()
    target_link_libraries(Grindstone ${CMAKE_DL_LIBS} BulletDynamics BulletCollision LinearMath)
endif()

target_include_directories(Grindstone PUBLIC ${DEPS_DIR} ${RAPIDJSON_INCLUDE_DIRS} ${BULLET_ROOT_DIR}/${BULLET_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})