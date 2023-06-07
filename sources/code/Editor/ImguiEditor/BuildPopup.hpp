#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class BuildPopup {
			public:
				void StartBuild();
				void Render();
			private:
				void CopyExecutableFile(std::string filename);
				void CopyEngineDLLFile(std::string filename);
				void CopyProjectDLLFile(std::string filename);
				void CopyPlugins();
				void CopyBinaries();
				void CopyMetaData();
				void CopyFile(std::filesystem::path srcBase, std::filesystem::path dstBase, const char* file);
				void CopyCompiledAssets();
				bool isShowing;
				std::filesystem::path targetPath;
				std::filesystem::path engineBinPath;
				std::filesystem::path projectBinPath;
				std::filesystem::path targetBinPath;
				std::filesystem::path sourceCompiledAssetsPath;
				std::filesystem::path targetCompiledAssetsPath;
				std::filesystem::path sourceBuildSettingsPath;
				std::filesystem::path targetBuildSettingsPath;
			};
		}
	}
}
