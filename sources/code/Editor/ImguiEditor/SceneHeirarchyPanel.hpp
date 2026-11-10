#pragma once

#include <entt/entity/fwd.hpp>

#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/ECS/Entity.hpp>

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
					entt::sigh_mixin<entt::basic_storage<entt::entity, entt::entity, std::allocator<entt::entity>>>,
					entt::sigh_mixin<entt::basic_storage<Grindstone::TagComponent, entt::entity, std::allocator<Grindstone::TagComponent>>>,
					entt::sigh_mixin<entt::basic_storage<Grindstone::ParentComponent, entt::entity, std::allocator<Grindstone::ParentComponent>>>
				>,
				entt::exclude_t<>
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
