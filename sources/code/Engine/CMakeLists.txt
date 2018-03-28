set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENGINE_CORE_HEADERS Core/exception.hpp Core/Engine.hpp Core/Entity.hpp Core/LoadingScreen.hpp Core/iniHandler.hpp Core/Input.hpp Core/InputInterface.hpp Core/Space.hpp Core/Utilities.hpp Core/LevelLoader.hpp)
source_group("Core\\Header Files" FILES ${ENGINE_CORE_HEADERS})
set(ENGINE_RENDERPATHS_HEADERS Renderpaths/RenderPath.hpp  Renderpaths/RenderPathDeferred.hpp Renderpaths/RenderPathForward.hpp)
source_group("Render Paths\\Header Files" FILES ${ENGINE_RENDERPATHS_HEADERS})
set(ENGINE_SYSTEMS_HEADERS Systems/Debug.hpp Systems/Skybox.hpp Systems/SController.hpp Systems/STransform.hpp Systems/SAudio.hpp Systems/CBase.hpp Systems/SCamera.hpp Systems/SPhysics.hpp Systems/SMaterial.hpp Systems/SAudio.hpp Systems/SCubemap.hpp Systems/SGameplay.hpp Systems/SGeometryStatic.hpp Systems/SGeometrySkeletal.hpp Systems/SGeometryTerrain.hpp Systems/SGeometry.hpp Systems/SLight.hpp)
source_group("Systems\\Header Files" FILES ${ENGINE_SYSTEMS_HEADERS})
set(ENGINE_POST_HEADERS PostProcess/BasePost.hpp PostProcess/PostPipeline.hpp PostProcess/ColorGradingPost.hpp PostProcess/PostProcessIBL.hpp PostProcess/PostProcessSSAO.hpp PostProcess/PostProcessSSAOBlur.hpp PostProcess/PostProcessTonemap.hpp)
source_group("Post Processing\\Header Files" FILES ${ENGINE_POST_HEADERS})
set(ENGINE_OTHER_HEADERS ../FormatCommon/Bounding.hpp ../FormatCommon/StaticModel.hpp)
source_group("Other\\Header Files" FILES ${ENGINE_OTHER_HEADERS})
set(ENGINE_HEADERS ${ENGINE_OTHER_HEADERS} ${ENGINE_CORE_HEADERS} ${ENGINE_ENTITES_HEADERS} ${ENGINE_RENDERPATHS_HEADERS} ${ENGINE_SYSTEMS_HEADERS} ${ENGINE_POST_HEADERS})

set(ENGINE_CORE_SOURCES Core/exception.cpp Core/Main.cpp Core/Entity.cpp Core/Engine.cpp Core/LoadingScreen.cpp Core/iniHandler.cpp Core/Input.cpp Core/Space.cpp Core/Utilities.cpp Core/LevelLoader.cpp)
source_group("Core\\Source Files" FILES ${ENGINE_CORE_SOURCES})
set(ENGINE_RENDERPATHS_SOURCES Renderpaths/RenderPathDeferred.cpp Renderpaths/RenderPathForward.cpp)
source_group("Render Paths\\Source Files" FILES ${ENGINE_RENDERPATHS_SOURCES})
set(ENGINE_SYSTEMS_SOURCES Systems/Debug.cpp Systems/Skybox.cpp Systems/SController.cpp Systems/STransform.cpp Systems/SAudio.cpp Systems/SCamera.cpp Systems/SPhysics.cpp Systems/SMaterial.cpp Systems/SAudio.cpp Systems/SCubemap.cpp Systems/SGameplay.cpp Systems/SGeometryStatic.cpp Systems/SGeometrySkeletal.cpp Systems/SGeometryTerrain.cpp Systems/SGeometry.cpp Systems/SLight.cpp)
source_group("Systems\\Source Files" FILES ${ENGINE_SYSTEMS_SOURCES})
set(ENGINE_POST_SOURCES PostProcess/PostPipeline.cpp PostProcess/ColorGradingPost.cpp PostProcess/PostProcessIBL.cpp PostProcess/PostProcessSSAO.cpp PostProcess/PostProcessSSAOBlur.cpp PostProcess/PostProcessTonemap.cpp)
source_group("Post Processing\\Source Files" FILES ${ENGINE_POST_SOURCES})
set(ENGINE_OTHER_SOURCES ../FormatCommon/Bounding.cpp)
source_group("Other\\Source Files" FILES ${ENGINE_OTHER_SOURCES})
set(ENGINE_SOURCES ${ENGINE_OTHER_SOURCES} ${ENGINE_CORE_SOURCES} ${ENGINE_ENTITES_SOURCES} ${ENGINE_RENDERPATHS_SOURCES} ${ENGINE_SYSTEMS_SOURCES} ${ENGINE_POST_SOURCES}) 

set(INCLUDE_DIRECTORIES ../GraphicsCommon ../AudioModule)

add_executable(Grindstone ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_compile_definitions(Grindstone PRIVATE _CRT_SECURE_NO_WARNINGS)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set_target_properties(Grindstone PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if (WIN32)
    #target_link_libraries(Grindstone optimized ${bullet_LIB_REL})
	target_link_libraries(Grindstone ${bullet_LIB_DEB})
else ()
    find_package (Threads)
    target_link_libraries(Grindstone ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} BulletDynamics BulletCollision LinearMath)
endif()

target_include_directories(Grindstone PUBLIC ${DEPS_DIR} ${RAPIDJSON_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})