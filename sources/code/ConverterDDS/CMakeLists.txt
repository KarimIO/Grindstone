set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CD_SOURCES main.cpp)

source_group("Header Files" FILES ${CD_HEADERS})
source_group("Source Files" FILES ${CD_SOURCES})

add_executable(converterDDS ${CD_SOURCES} ${CD_HEADERS})

set_target_properties(converterDDS PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_include_directories(converterDDS PRIVATE ${DEPS_DIR})