set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PLUGIN_XML_UI_HEADERS PluginXmlUi.hpp XmlUiSystem.hpp Stylesheet.hpp Style.hpp)
set(PLUGIN_XML_UI_SOURCES PluginXmlUi.cpp XmlUiSystem.cpp Stylesheet.cpp ../../deps/pugixml/src/pugixml.cpp)

add_executable(PluginXmlUi ${PLUGIN_XML_UI_SOURCES} ${PLUGIN_XML_UI_HEADERS})
target_compile_definitions(PluginXmlUi PRIVATE PLUGIN_XML_UI)
set_target_properties(PluginXmlUi PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")