#pragma once

#include <vector>
#include <string>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ScriptBuilder {
			class SolutionBuilder {
			public:
				void AddProject(CSharpProjectMetaData& projectMetaData);
				void CreateSolution();
			private:
				void OutputFile(std::string& output);
				void WriteMainProjectSection(std::string& output);
				void WriteSolutionConfigs(std::string& output);
				void WriteProjectConfigs(std::string& output);
				void WriteSolutionProperties(std::string& output);
				
				std::vector<CSharpProjectMetaData> projects;
			};
		}
	}
}
