set(APPL_EXEC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_MAIN Main.cpp ../Common/Utilities/ModuleLoading.cpp)

add_executable(ApplicationExecutable ${SOURCE_MAIN})

set(APPLICATION_NAME "Application" CACHE STRING "Name of the output Executable")
set_target_properties(ApplicationExecutable PROPERTIES OUTPUT_NAME "Application")

set_property(TARGET ApplicationExecutable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BUILD_DIRECTORY})

target_include_directories(ApplicationExecutable
    PUBLIC ../
)

target_link_libraries(ApplicationExecutable ${CMAKE_DL_LIBS})
