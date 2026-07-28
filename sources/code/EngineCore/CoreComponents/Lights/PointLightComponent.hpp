#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"
#include "Common/Rect.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class Buffer;
		class RenderPass;
		class Framebuffer;
		class DepthStencilTarget;
		class DescriptorSet;
		class DescriptorSetLayout;
	}

	struct PointLightComponent {
		struct ShadowData {
			Grindstone::Math::Matrix4 shadowMatrix;
			Grindstone::Math::Rect2D shadowRenderArea;
		};

		struct UniformStruct {
			ShadowData shadowData[6];
			glm::vec3 lightPosition = glm::vec3(1, 2, 1);
			float lightAttenuationRadius = 40.0f;
			glm::vec3 lightColor = glm::vec3(3, 0.8, 0.4);
			float dummy;
		};

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		ShadowData shadowData[6];
		Math::Float3 color = Math::Float3(1.0f, 0.8f, 0.6f);
		float attenuationRadius = 16.0f;
		float intensity = 16.0f;

		GraphicsAPI::Buffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::Buffer* shadowMapUniformBufferObjects[6];
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSets[6];
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("PointLight")
	};
}
