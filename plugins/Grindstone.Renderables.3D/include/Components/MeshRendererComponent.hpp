#pragma once

#include <string>
#include <vector>

#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/Assets/Materials/MaterialAsset.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Buffer;
		class DescriptorSet;
	}

	struct MeshRendererComponent {
		std::vector<AssetReference<MaterialAsset>> materials;

		static void Construct(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);

		GraphicsAPI::Buffer* perDrawUniformBuffer = nullptr;
		GraphicsAPI::DescriptorSet* perDrawDescriptorSet = nullptr;

		REFLECT("MeshRenderer")
	};
}
