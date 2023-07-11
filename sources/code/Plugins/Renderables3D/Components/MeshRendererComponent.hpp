#pragma once

#include <string>
#include <vector>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Materials/MaterialAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class DescriptorSet;
	}

	struct MeshRendererComponent {
		std::vector<AssetReference<MaterialAsset>> materials;

		GraphicsAPI::UniformBuffer* perDrawUniformBuffer = nullptr;
		GraphicsAPI::DescriptorSet* perDrawDescriptorSet = nullptr;

		REFLECT("MeshRenderer")
	};
}
