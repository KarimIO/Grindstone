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

	struct SpotLightComponent {
		struct UniformStruct {
			Math::Matrix4 shadowMatrix;
			Math::Float3 color;
			float attenuationRadius;
			Math::Float3 position;
			float intensity;
			Math::Float3 direction;
			float innerAngle;
			float outerAngle;
			float shadowResolution;
		};

		Math::Matrix4 shadowMatrix;
		Math::Float3 color;
		float attenuationRadius;
		float intensity;
		float innerAngle;
		float outerAngle;
		float shadowResolution;

		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		REFLECT("SpotLight")
	};

	void SetupSpotLightComponent(ECS::Entity& entity, void* componentPtr);
}
