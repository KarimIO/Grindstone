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
				CSharpProjectBuilder(const CSharpProjectMetaData& metaData);
				void AddCodeFile(const std::filesystem::path& fileName);
				void AddNonCodeFile(const std::filesystem::path& fileName);
				void CreateProject() const;
			private:
				void OutputFile(const std::string& output) const;
				void WriteProjectInfo(std::string& output) const;
				void WriteCodeFiles(std::string& output) const;
				static void WriteDllReferenceByFilename(std::string& output, const std::string& path);
				static void WriteDllReference(std::string& output, const std::string& path);
				static void WriteTargets(std::string& output);
				
				std::string assemblyName;
				std::string guid;
				std::vector<std::filesystem::path> codeFiles;
				std::vector<std::filesystem::path> nonCodeFiles;
			};
		}
	}
}
