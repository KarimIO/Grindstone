#pragma once

#include <vector>

#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::BuildSettings {
	class SceneBuildSettings {
	public:
		SceneBuildSettings();
		void Load();
		Grindstone::Uuid GetDefaultScene() const;
	private:
		std::vector<Grindstone::Uuid> sceneUuids;
	};
}
