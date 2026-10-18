install(
	FILES "plugins/Grindstone.Editor.Assets/CMakeLists.txt"
		"plugins/Grindstone.Editor.Assets/plugin.meta.json"
		"plugins/Grindstone.Editor.Assets/README.md"
	DESTINATION plugins/Grindstone.Editor.Assets/
)

file(GLOB DEPLOY_FILES_AND_DIRS "plugins/Grindstone.Editor.Assets/assets/*")
foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
   IF( IS_DIRECTORY "${ITEM}" )
      LIST( APPEND DIRS_TO_DEPLOY "${ITEM}" )
   ELSE()
      LIST( APPEND FILES_TO_DEPLOY "${ITEM}" )
   ENDIF()
endforeach()
INSTALL( FILES ${FILES_TO_DEPLOY} DESTINATION plugins/Grindstone.Editor.Assets/ )
INSTALL( DIRECTORY ${DIRS_TO_DEPLOY} DESTINATION plugins/Grindstone.Editor.Assets/ )
