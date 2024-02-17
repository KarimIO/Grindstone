#pragma once

#include <vector>
#include <string>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ScriptBuilder {
			class SolutionBuilder {
			public:
				void AddProject(const CSharpProjectMetaData& projectMetaData);
				void CreateSolution();
			private:
				static void OutputFile(const std::string& output);
				void WriteMainProjectSection(std::string& output) const;
				void WriteSolutionConfigs(std::string& output);
				void WriteProjectConfigs(std::string& output);
				void WriteSolutionProperties(std::string& output);

				std::vector<CSharpProjectMetaData> projects;
			};
		}
	}
}
