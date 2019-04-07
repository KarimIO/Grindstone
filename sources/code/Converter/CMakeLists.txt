set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CV_SOURCES ../AssetCommon/SkeletonLoader.cpp ImageConverter.cpp ModelConverter.cpp AnimationConverter.cpp MaterialCreator.cpp Utilities.cpp ../FormatCommon/Bounding.cpp Main.cpp)
set(CV_HEADERS ../AssetCommon/SkeletonLoader.hpp ImageConverter.hpp ModelConverter.hpp AnimationConverter.hpp MaterialCreator.hpp Utilities.hpp ../FormatCommon/Bounding.hpp ../FormatCommon/StaticModel.hpp)

source_group("Header Files" FILES ${CV_HEADERS})
source_group("Source Files" FILES ${CV_SOURCES})
link_directories(${ASSIMP_LIBRARY_DIR})
add_executable(converter ${CV_SOURCES} ${CV_HEADERS})

set_target_properties(converter PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_include_directories(converter PRIVATE ${ASSIMP_INCLUDE_DIR} ${DEPS_DIR} ${DEPS_DIR}/spdlog/include/ ${CMAKE_CURRENT_SOURCE_DIR}/../Engine/)
target_link_libraries(converter ${assimp_LIBRARIES})