cmake_minimum_required (VERSION 3.2)
PROJECT(Grindstone C CXX)

enable_testing()
set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/sources/cmake)
message(STATUS ${CMAKE_MODULE_PATH})
find_package(assimp REQUIRED)
find_package(Bullet REQUIRED)
find_package(OpenAL REQUIRED)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources/deps")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} ) 
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG ) 
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_SOURCE_DIR}/bin ) 
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_SOURCE_DIR}/bin ) 
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_SOURCE_DIR}/bin ) 
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES ) 

add_subdirectory(sources/code/Engine)
add_subdirectory(sources/code/Converter)
add_subdirectory(sources/code/AudioOpenAL)

option(USE_OPENGL "Should OpenGL be compiled?" ON)

if (WIN32)
    option(USE_DIRECTX "Should DirectX be compiled?" ON)

    if (USE_DIRECTX)
        message(STATUS "Preparing DirectX project")
        add_subdirectory(sources/code/GraphicsDirectX)
    endif(USE_DIRECTX)
endif(WIN32)

if (NOT APPLE)
    option(USE_VULKAN "Should Vulkan be compiled?" OFF)
    if (USE_VULKAN)
        find_package(FindVulkan)

        if (Vulkan_FOUND)
            message(STATUS "Preparing Vulkan project")
            add_subdirectory(sources/code/GraphicsVulkan)
        endif(Vulkan_FOUND)
    endif(USE_VULKAN)
endif(NOT APPLE)

if (USE_OPENGL)
    message(STATUS "Preparing OpenGL project")
    add_subdirectory(sources/code/GraphicsOpenGL)
endif(USE_OPENGL)
