#pragma once

#include <string>

namespace Grindstone::Editor::ScriptBuilder {
	struct CSharpProjectMetaData {
		CSharpProjectMetaData(
			const std::string& name,
			const std::string& guid
		) : assemblyName(name),
		    assemblyGuid(guid) {}

		std::string assemblyName;
		std::string assemblyGuid;
	};
}
