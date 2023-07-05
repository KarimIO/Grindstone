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

	struct PointLightComponent {
		struct UniformStruct {
			glm::vec3 lightColor = glm::vec3(3, 0.8, 0.4);
			float lightAttenuationRadius = 40.0f;
			glm::vec3 lightPosition = glm::vec3(1, 2, 1);
			float lightIntensity = 40.0f;
		};

		Math::Float3 color;
		float attenuationRadius;
		float intensity;

		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		REFLECT("PointLight")
	};

	void SetupPointLightComponent(ECS::Entity& entity, void* componentPtr);
}
