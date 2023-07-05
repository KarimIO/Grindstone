#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class DescriptorSet;
		class DescriptorSetLayout;
	}

	struct DirectionalLightComponent {
		struct UniformStruct {
			Math::Matrix4 shadowMatrix;
			Math::Float3 color;
			float sourceRadius;
			Math::Float3 direction;
			float intensity;
			float shadowResolution;
		};

		Math::Float3 color;
		float sourceRadius;
		float intensity;
		float shadowResolution;

		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		REFLECT("DirectionalLight")
	};

	void SetupDirectionalLightComponent(ECS::Entity& entity, void* componentPtr);
}
