set(COMMON_INCLUDES ${COMMON_DIR}/include)
set(COMMON_BIN ${COMMON_DIR}/bin)
set(COMMON_LIB ${COMMON_DIR}/lib)
set(SRC ${COMMON_DIR}/source)
set(INC ${COMMON_DIR}/include)

file(
    GLOB_RECURSE SOURCE_MAIN
    ${COMMON_DIR}/Assert.hpp
    ${COMMON_DIR}/Break.hpp
    ${COMMON_DIR}/Buffer.hpp
    ${COMMON_DIR}/EnumTraits.hpp
    ${COMMON_DIR}/Filepath.hpp
    ${COMMON_DIR}/GameplayTag.cpp
    ${COMMON_DIR}/GameplayTag.hpp
    ${COMMON_DIR}/Hash.hpp
    ${COMMON_DIR}/HashedString.cpp
    ${COMMON_DIR}/HashedString.hpp
    ${COMMON_DIR}/IntTypes.hpp
    ${COMMON_DIR}/Logging.hpp
    ${COMMON_DIR}/Math.hpp
    ${COMMON_DIR}/Observable.hpp
    ${COMMON_DIR}/String.hpp
)

file(GLOB_RECURSE SOURCE_ASSETS "${COMMON_DIR}/Assets/*.cpp")
file(GLOB_RECURSE HEADER_ASSETS "${COMMON_DIR}/Assets/*.hpp")
source_group("Source Files\\Assets" FILES ${SOURCE_ASSETS})
source_group("Header Files\\Assets" FILES ${HEADER_ASSETS})

file(GLOB_RECURSE SOURCE_CONSOLE "${COMMON_DIR}/Console/*.cpp")
file(GLOB_RECURSE HEADER_CONSOLE "${COMMON_DIR}/Console/*.hpp")
source_group("Source Files\\Console" FILES ${SOURCE_CONSOLE})
source_group("Header Files\\Console" FILES ${HEADER_CONSOLE})

file(GLOB_RECURSE SOURCE_CONTAINERS "${COMMON_DIR}/Containers/*.cpp")
file(GLOB_RECURSE HEADER_CONTAINERS "${COMMON_DIR}/Containers/*.hpp")
source_group("Source Files\\Containers" FILES ${SOURCE_CONTAINERS})
source_group("Header Files\\Containers" FILES ${HEADER_CONTAINERS})

file(GLOB_RECURSE SOURCE_DISPLAY "${COMMON_DIR}/Display/*.cpp")
file(GLOB_RECURSE HEADER_DISPLAY "${COMMON_DIR}/Display/*.hpp")
source_group("Source Files\\Display" FILES ${SOURCE_DISPLAY})
source_group("Header Files\\Display" FILES ${HEADER_DISPLAY})

file(GLOB_RECURSE SOURCE_EVENT "${COMMON_DIR}/Event/*.cpp")
file(GLOB_RECURSE HEADER_EVENT "${COMMON_DIR}/Event/*.hpp")
source_group("Source Files\\Event" FILES ${SOURCE_EVENT})
source_group("Header Files\\Event" FILES ${HEADER_EVENT})

file(GLOB_RECURSE SOURCE_FORMATS "${COMMON_DIR}/Formats/*.cpp")
file(GLOB_RECURSE HEADER_FORMATS "${COMMON_DIR}/Formats/*.hpp")
source_group("Source Files\\Formats" FILES ${SOURCE_FORMATS})
source_group("Header Files\\Formats" FILES ${HEADER_FORMATS})

file(GLOB_RECURSE SOURCE_GRAPHICS "${COMMON_DIR}/Graphics/*.cpp")
file(GLOB_RECURSE HEADER_GRAPHICS "${COMMON_DIR}/Graphics/*.hpp")
source_group("Source Files\\Graphics" FILES ${SOURCE_GRAPHICS})
source_group("Header Files\\Graphics" FILES ${HEADER_GRAPHICS})

file(GLOB_RECURSE SOURCE_INPUT "${COMMON_DIR}/Input/*.cpp")
file(GLOB_RECURSE HEADER_INPUT "${COMMON_DIR}/Input/*.hpp")
source_group("Source Files\\Input" FILES ${SOURCE_INPUT})
source_group("Header Files\\Input" FILES ${HEADER_INPUT})

file(GLOB_RECURSE SOURCE_MEMORY "${COMMON_DIR}/Memory/*.cpp")
file(GLOB_RECURSE HEADER_MEMORY "${COMMON_DIR}/Memory/*.hpp")
source_group("Source Files\\Memory" FILES ${SOURCE_MEMORY})
source_group("Header Files\\Memory" FILES ${HEADER_MEMORY})

file(GLOB_RECURSE SOURCE_RENDERING "${COMMON_DIR}/Rendering/*.cpp")
file(GLOB_RECURSE HEADER_RENDERING "${COMMON_DIR}/Rendering/*.hpp")
source_group("Source Files\\Rendering" FILES ${SOURCE_RENDERING})
source_group("Header Files\\Rendering" FILES ${HEADER_RENDERING})

file(GLOB_RECURSE SOURCE_RESOURCE_PIPELINE "${COMMON_DIR}/ResourcePipeline/*.cpp")
file(GLOB_RECURSE HEADER_RESOURCE_PIPELINE "${COMMON_DIR}/ResourcePipeline/*.hpp")
source_group("Source Files\\ResourcePipeline" FILES ${SOURCE_RESOURCE_PIPELINE})
source_group("Header Files\\ResourcePipeline" FILES ${HEADER_RESOURCE_PIPELINE})

file(GLOB_RECURSE SOURCE_UTILITIES "${COMMON_DIR}/Utilities/*.cpp")
file(GLOB_RECURSE HEADER_UTILITIES "${COMMON_DIR}/Utilities/*.hpp")
source_group("Source Files\\Utilities" FILES ${SOURCE_UTILITIES})
source_group("Header Files\\Utilities" FILES ${HEADER_UTILITIES})

file(GLOB_RECURSE SOURCE_WINDOW "${COMMON_DIR}/Window/*.cpp")
file(GLOB_RECURSE HEADER_WINDOW "${COMMON_DIR}/Window/*.hpp")
source_group("Source Files\\Window" FILES ${SOURCE_WINDOW})
source_group("Header Files\\Window" FILES ${HEADER_WINDOW})

set(COMMON_SOURCES
    ${SOURCE_MAIN}
    ${SOURCE_ASSETS}
    ${SOURCE_CONSOLE}
    ${SOURCE_CONTAINERS}
    ${SOURCE_DISPLAY}
    ${SOURCE_EVENT}
    ${SOURCE_FORMATS}
    ${SOURCE_GRAPHICS}
    ${SOURCE_INPUT}
    ${SOURCE_MEMORY}
    ${SOURCE_RENDERING}
    ${SOURCE_RESOURCE_PIPELINE}
    ${SOURCE_UTILITIES}
    ${SOURCE_WINDOW}
)

set(COMMON_HEADERS
    ${HEADER_ASSETS}
    ${HEADER_CONSOLE}
    ${HEADER_CONTAINERS}
    ${HEADER_DISPLAY}
    ${HEADER_EVENT}
    ${HEADER_FORMATS}
    ${HEADER_GRAPHICS}
    ${HEADER_INPUT}
    ${HEADER_MEMORY}
    ${HEADER_RENDERING}
    ${HEADER_RESOURCE_PIPELINE}
    ${HEADER_UTILITIES}
    ${HEADER_WINDOW}
)

add_library(Common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
find_package(glfw3 CONFIG REQUIRED)

set_target_properties(Common PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${COMMON_BIN}"
	LIBRARY_OUTPUT_DIRECTORY "${COMMON_LIB}"
	ARCHIVE_OUTPUT_DIRECTORY "${COMMON_LIB}"
)
set_property(TARGET Common PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_features(Common PRIVATE cxx_std_20)
target_link_libraries(Common glfw)

target_include_directories(Common PUBLIC ${COMMON_DIR} ${CODE_DIR} ${PLUGIN_DIR})
target_compile_definitions(Common PUBLIC NOMINMAX GLM_ENABLE_EXPERIMENTAL)

message(STATUS
"Common provides CMake targets:

- COMMON_DIR => ${COMMON_DIR}
- COMMON_INCLUDES => ${COMMON_INCLUDES}
- COMMON_BIN => ${COMMON_BIN}
- COMMON_LIB => ${COMMON_LIB}

")
