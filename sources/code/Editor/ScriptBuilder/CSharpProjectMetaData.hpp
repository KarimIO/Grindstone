#pragma once

#include <string>

namespace Grindstone {
	namespace Editor {
		namespace ScriptBuilder {
			struct CSharpProjectMetaData {
				CSharpProjectMetaData(
					std::string name,
					std::string guid
				) : assemblyName(name),
					assemblyGuid(guid) {}

				std::string assemblyName;
				std::string assemblyGuid;
			};
		}
	}
}
