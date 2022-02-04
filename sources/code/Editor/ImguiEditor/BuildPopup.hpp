#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class BuildPopup {
			public:
				void StartBuild();
				void Render();
			private:
				void CopyBuildFile(std::string filename);
				void CopyExecutableFile(std::string filename);
				void CopyDLLFile(std::string filename);
				void CopyPlugins();
				void CopyBinaries();
				void CopyMetaData();
				void CopyCompiledAssets();
				bool isShowing;
				std::filesystem::path sourceBuildPath;
				std::filesystem::path targetPath;
				std::filesystem::path targetBuildPath;
			};
		}
	}
}
