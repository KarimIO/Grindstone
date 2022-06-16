#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ScriptBuilder {
			class CSharpProjectBuilder {
			public:
				CSharpProjectBuilder(CSharpProjectMetaData project);
				void AddCodeFile(std::filesystem::path& fileName);
				void AddNonCodeFile(std::filesystem::path& fileName);
				void CreateProject();
			private:
				void OutputFile(std::string& output);
				void WriteProjectInfo(std::string& output);
				void WriteCodeFiles(std::string& output);
				void WriteDllReferenceByFilename(std::string& output, std::string path);
				void WriteDllReference(std::string& output, std::string path);
				void WriteTargets(std::string& output);
				
				std::string assemblyName;
				std::string guid;
				std::vector<std::filesystem::path> codeFiles;
				std::vector<std::filesystem::path> nonCodeFiles;
			};
		}
	}
}
