#pragma once

#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Parent/ParentComponent.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;


			using EntityParentTagView = entt::basic_view<
				entt::get_t<
				entt::sigh_mixin<entt::basic_storage<entt::entity, entt::entity, std::allocator<entt::entity>, void>>,
				entt::sigh_mixin<entt::basic_storage<Grindstone::TagComponent, entt::entity, std::allocator<Grindstone::TagComponent>, void>>,
				entt::sigh_mixin<entt::basic_storage<Grindstone::ParentComponent, entt::entity, std::allocator<Grindstone::ParentComponent>, void>>
				>,
				entt::exclude_t<>,
				void
			>;

			class SceneHeirarchyPanel {
			public:
				SceneHeirarchyPanel(SceneManagement::SceneManager* sceneManager, ImguiEditor* editor);
				void Render();
			private:
				void RenderScene(SceneManagement::Scene* scene);
				void RenderEntity(
					EntityParentTagView& view,
					ECS::Entity entity,
					TagComponent& tagComponent,
					ParentComponent& parentComponent
				);
			private:
				bool isShowingPanel = true;
				SceneManagement::SceneManager* sceneManager;
				ImguiEditor* editor;
				ECS::Entity entityToRename;
				std::string entityRenameNewName;
			};
		}
	}
}
