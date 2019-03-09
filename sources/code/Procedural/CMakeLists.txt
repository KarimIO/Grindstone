set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROC_SOURCES BuildHeightmapNode.cpp ErosionNode.cpp FastNoise.cpp PerlinNoiseGenerator.cpp Main.cpp)
set(PROC_HEADERS BuildHeightmapNode.hpp ErosionNode.hpp FastNoise.hpp PerlinNoiseGenerator.hpp ../../deps/stb/stb_image_write.h)

source_group("Header Files" FILES ${PROC_HEADERS})
source_group("Source Files" FILES ${PROC_SOURCES})
add_executable(procedural_generator ${PROC_SOURCES} ${PROC_HEADERS})

set_target_properties(procedural_generator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_include_directories(procedural_generator PRIVATE ${DEPS_DIR})
target_link_libraries(procedural_generator ${assimp_LIBRARIES})