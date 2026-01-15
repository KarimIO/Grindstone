#pragma once

#include <vector>
#include <string>

#include <Common/ResourcePipeline/Uuid.hpp>

#include "../Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class Build : public BasePage {
	public:
		virtual void Open() override;
		virtual void Render() override;
		virtual void Save() override;
		virtual void Reset() override;

		struct SceneData {
			Grindstone::Uuid uuid;
			std::string displayName;

			SceneData() = default;
			SceneData(
				Grindstone::Uuid uuid,
				std::string displayName
			) : uuid(uuid), displayName(displayName) {}
		};

	protected:
		std::vector<SceneData> sceneList;
	};
}
