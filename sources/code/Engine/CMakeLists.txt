set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ENGINE_CORE_HEADERS Core/Engine.h Core/EBase.h Core/GraphicsDLLPointer.h Core/iniHandler.h Core/Input.h Core/InputInterface.h Core/TextureManager.h  Core/Space.h Core/Utilities.h Core/LevelLoader.h)
source_group("Core\\Header Files" FILES ${ENGINE_CORE_HEADERS})
set(ENGINE_RENDERPATHS_HEADERS RenderPaths/RenderPath.h  RenderPaths/RenderPathDeferred.h RenderPaths/RenderPathForward.h)
source_group("Render Paths\\Header Files" FILES ${ENGINE_RENDERPATHS_HEADERS})
set(ENGINE_SYSTEMS_HEADERS Systems/CController.h Systems/CTransform.h Systems/CAudio.h Systems/CBase.h Systems/CCamera.h Systems/CLight.h Systems/Physics.h Systems/SAudio.h Systems/SCubemap.h Systems/SGeometry.h Systems/SLight.h Systems/Terrain.h)
source_group("Systems\\Header Files" FILES ${ENGINE_SYSTEMS_HEADERS})
set(ENGINE_HEADERS ${ENGINE_CORE_HEADERS} ${ENGINE_ENTITES_HEADERS} ${ENGINE_RENDERPATHS_HEADERS} ${ENGINE_SYSTEMS_HEADERS})

set(ENGINE_CORE_SOURCES Core/Main.cpp Core/EBase.cpp Core/Engine.cpp Core/iniHandler.cpp Core/Input.cpp Core/TextureManager.cpp Core/Space.cpp Core/Utilities.cpp Core/LevelLoader.cpp)
source_group("Core\\Source Files" FILES ${ENGINE_CORE_SOURCES})
set(ENGINE_RENDERPATHS_SOURCES RenderPaths/RenderPathDeferred.cpp RenderPaths/RenderPathForward.cpp)
source_group("Render Paths\\Source Files" FILES ${ENGINE_RENDERPATHS_SOURCES})
set(ENGINE_SYSTEMS_SOURCES Systems/CController.cpp Systems/CTransform.cpp Systems/CAudio.cpp Systems/CBase.cpp Systems/CCamera.cpp Systems/CLight.cpp Systems/Physics.cpp Systems/SAudio.cpp Systems/SCubemap.cpp Systems/SGeometry.cpp Systems/SLight.cpp Systems/Terrain.cpp)
source_group("Systems\\Source Files" FILES ${ENGINE_SYSTEMS_SOURCES})
set(ENGINE_SOURCES ${ENGINE_CORE_SOURCES} ${ENGINE_ENTITES_SOURCES} ${ENGINE_RENDERPATHS_SOURCES} ${ENGINE_SYSTEMS_SOURCES})

#ASSIMP
set(ASSIMP_INC "D:/Libraries/Assimp/include" CACHE PATH "Location of Assimp Include")
include_directories(${ASSIMP_INC})

set(ASSIMP_LIB "D:/Libraries/Assimp/lib64" CACHE PATH "Location of Assimp Library")
link_directories(${ASSIMP_LIB})

#BULLET
#set(BULLET_DIR "D:/Libraries/Bullet" CACHE PATH "Location of Bullet Library")
#include_directories(${BULLET_DIR}/src)
#link_directories(${BULLET_DIR}/bin)

#rapidJSON
set(RAPIDJSON_DIR "D:/Libraries/rapidjson-master/include/" CACHE PATH "Location of rapidJSON Includes folder")
include_directories(${RAPIDJSON_DIR})

include_directories(${GLM_DIR})
include_directories(${CMAKE_SOURCE_DIRECTORY}/sources/code/Engine/)


set(STB_DIR "D:/Libraries/STB" CACHE PATH "Location of STB Image Library")
include_directories(${STB_DIR})

add_executable(engine ${ENGINE_SOURCES} ${ENGINE_HEADERS})
set_target_properties(engine PROPERTIES OUTPUT_NAME "Grindstone")

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set_target_properties(engine PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY_DEBUG "${OUTPUT_DIR}")
endif()

include_directories(${OUTPUT_DIR}/../sources/code/AudioModule/)
include_directories(${OUTPUT_DIR}/../sources/code/WindowModule/)
include_directories(${OUTPUT_DIR}/../sources/code/GraphicsCommon/)
include_directories(${OUTPUT_DIR}/../sources/code/GraphicsOpenGL/)

target_compile_definitions(engine PRIVATE _CRT_SECURE_NO_WARNINGS)

target_link_libraries(engine assimp)
if(false) #WIN32)
    target_link_libraries(engine debug BulletCollision_x64_debug)
    target_link_libraries(engine debug BulletDynamics_x64_debug)
    target_link_libraries(engine debug LinearMath_x64_debug)

    target_link_libraries(engine optimized BulletCollision_x64_release)
    target_link_libraries(engine optimized BulletDynamics_x64_release)
    target_link_libraries(engine optimized LinearMath_x64_release)
else()
    #target_link_libraries(engine libBulletCollision)
    #target_link_libraries(engine libBulletDynamics)
    #target_link_libraries(engine libLinearMath)
endif()