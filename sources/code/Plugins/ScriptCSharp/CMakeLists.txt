function(get_current_dotnet_rid RID_OUT)
	if(DEFINED ENV{DOTNET_RUNTIME_ID})
		set(RID "$ENV{DOTNET_RUNTIME_ID}")
	else()
		string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH)

		if(ARCH STREQUAL "amd64" OR ARCH STREQUAL "x86_64")
			set(ARCH "x64")
		elseif(ARCH MATCHES "^(arm64|aarch64)$")
			set(ARCH "arm64")
		else()
			message(FATAL_ERROR "Unsupported architecture for RID: ${CMAKE_SYSTEM_PROCESSOR}")
		endif()

		if(WIN32)
			set(OS "win")
		elseif(APPLE)
			set(OS "osx")
		elseif(UNIX)
			set(OS "linux")
		else()
			message(FATAL_ERROR "Unsupported platform for RID")
		endif()

		set(RID "${OS}-${ARCH}")
	endif()

	set(${RID_OUT} "${RID}" PARENT_SCOPE)
endfunction()

function(get_dotnet_root DOTNET_ROOT_OUT)
	string(REGEX MATCH "Base Path:[ ]+([^\n\r]+)" _match "${dotnet_info}")
	if(NOT _match)
		message(FATAL_ERROR "Could not parse 'dotnet --info' for Base Path")
	endif()
	string(REGEX REPLACE "Base Path:[ ]+" "" dotnet_sdk_path "${_match}")
	get_filename_component(dotnet_root "${dotnet_sdk_path}" DIRECTORY)

	string(REGEX MATCH "Host:[ \n\r]+[ ]+Version:[ ]+([^\n\r]+)" _match "${dotnet_info}")
	if(NOT _match)
		message(FATAL_ERROR "Could not parse 'dotnet --info' for Version")
	endif()
	string(REGEX REPLACE "Host:[ \n\r]+[ ]+Version:[ ]+" "" dotnet_version "${_match}")

	get_current_dotnet_rid(RID)

	set(${DOTNET_ROOT_OUT} "${dotnet_root}/../packs/Microsoft.NETCore.App.Host.${RID}/${dotnet_version}/runtimes/${RID}/native" PARENT_SCOPE)
endfunction()

# =====================================================================
# Main
# =====================================================================
set(CSHARP_SOURCES CSharpManager.cpp CSharpSystem.cpp ScriptClass.cpp EntryPoint.cpp ../../EngineCore/ECS/Entity.cpp ../../EngineCore/Utils/Utilities.cpp)
set(CSHARP_HEADERS CSharpManager.hpp CSharpSystem.hpp ScriptClass.hpp)

file(GLOB_RECURSE CSHARP_COMPONENTS_SOURCES "Components/*.cpp")
file(GLOB_RECURSE CSHARP_COMPONENTS_HEADER "Components/*.hpp")
source_group("Components\\Source Files" FILES ${CSHARP_COMPONENTS_SOURCES})
source_group("Components\\Header Files" FILES ${CSHARP_COMPONENTS_HEADER})

source_group("Reflection\\Source Files" FILES ${SOURCE_REFLECTION})
source_group("Reflection\\Header Files" FILES ${HEADER_REFLECTION})

add_library(
	PluginScriptCSharp MODULE
	${CSHARP_SOURCES}
	${CSHARP_HEADERS}
	${CSHARP_COMPONENTS_SOURCES}
	${CSHARP_COMPONENTS_HEADER}
	${SOURCE_REFLECTION}
	${HEADER_REFLECTION}
	${CORE_UTILS}
)

target_compile_features(PluginScriptCSharp PRIVATE cxx_std_17)
set_target_properties(PluginScriptCSharp PROPERTIES FOLDER Plugins/Scripts)

find_path(ENTT_INCLUDE_DIRS "entt/entt.hpp")

execute_process(
	COMMAND dotnet --info
	OUTPUT_VARIABLE dotnet_info
	OUTPUT_STRIP_TRAILING_WHITESPACE
)


get_dotnet_root(DOTNET_ROOT)
if(EXISTS "${DOTNET_ROOT}")
	message(STATUS "Latest .NET SDK Version: ${DOTNET_ROOT}")
else()
	message(FATAL_ERROR ".NET SDK not found after searching: ${DOTNET_ROOT}")
endif()

target_include_directories(PluginScriptCSharp PUBLIC
	${CODE_DIR}
	${ENTT_INCLUDE_DIRS}
	${DOTNET_ROOT}
)
target_compile_definitions(PluginScriptCSharp PUBLIC CSHARP_DLL GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(PluginScriptCSharp PUBLIC ${CORE_LIBS} ${DOTNET_ROOT}/nethost.lib)
target_precompile_headers(PluginScriptCSharp PUBLIC pch.hpp)

file(COPY
	"${DOTNET_ROOT}/nethost.dll"
	DESTINATION "${BUILD_DIRECTORY}"
)
