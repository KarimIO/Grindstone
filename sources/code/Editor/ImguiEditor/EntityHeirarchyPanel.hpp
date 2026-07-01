#pragma once

#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Parent/ParentComponent.hpp"

namespace Grindstone::Editor::ImguiEditor {
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

	class EntityHeirarchyPanel {
	public:
		EntityHeirarchyPanel(ImguiEditor* editor);
		void Render();
	private:
		bool isShowingPanel = true;
		ImguiEditor* editor;
	};
}
