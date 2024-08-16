#pragma once

#include <string>
#include <map>

#include <Common/ResourcePipeline/Uuid.hpp>

#include "Scene.hpp"

namespace Grindstone::SceneManagement {
	class SceneManager {
	public:
		~SceneManager();

		void LoadDefaultScene();

		virtual void AddPostLoadProcess(std::function<void(Scene*)>);
		virtual void SaveScene(const std::filesystem::path& path, Scene* scene);
		virtual Scene* LoadSceneAdditively(Grindstone::Uuid);
		virtual Scene* LoadScene(Grindstone::Uuid);
		virtual Scene* CreateEmptyScene(const char* name);
		virtual Scene* CreateEmptySceneAdditively(const char* name);
		virtual void CloseActiveScenes();
		std::map<Grindstone::Uuid, Scene*> scenes;
	private:

		void ProcessSceneAfterLoading(Scene* scene);
		std::vector<std::function<void(Scene*)>> postLoadProcesses;
	};
}
