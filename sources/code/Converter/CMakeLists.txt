set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CV_SOURCES Main.cpp ModelConverter.cpp AnimationConverter.cpp MaterialCreator.cpp Utilities.cpp)
set(CV_HEADERS ModelConverter.h AnimationConverter.h MaterialCreator.h Utilities.h)

source_group("Header Files" FILES ${CV_HEADERS})
source_group("Source Files" FILES ${CV_SOURCES})

add_executable(converter ${CV_SOURCES} ${CV_HEADERS})

set_target_properties(converter PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_include_directories(converter PRIVATE ${ASSIMP_INCLUDE_DIRS})
link_directories(${ASSIMP_LIB_DIR})
target_link_libraries(converter assimp)