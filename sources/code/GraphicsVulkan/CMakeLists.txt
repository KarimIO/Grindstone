set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(VK_SOURCES VkGraphicsGeneral.cpp VkGraphicsWindows.cpp)
add_library(graphicsvk MODULE ${VK_SOURCES})

SET(VULKAN_DIR "D:/Libraries/Vulkan" CACHE PATH "Location of Vulkan Library")
include_directories(${VULKAN_DIR}/Include)
link_directories(${VULKAN_DIR}/Bin)

target_compile_definitions(graphicsvk PRIVATE GRAPHICS_DLL)

target_link_libraries(graphicsvk vulkan-1)