#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

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
		struct UniformStruct {
			glm::vec3 lightColor = glm::vec3(3, 0.8, 0.4);
			float lightAttenuationRadius = 40.0f;
			glm::vec3 lightPosition = glm::vec3(1, 2, 1);
			float lightIntensity = 40.0f;
			// float shadowResolution;
		};

		Math::Float3 color;
		float attenuationRadius;
		float intensity;

		GraphicsAPI::Buffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		/* TODO: Re-add this when you come back to point light shadows
		float shadowResolution;
		GraphicsAPI::RenderPass* renderPass = nullptr;
		GraphicsAPI::Framebuffer* framebuffer = nullptr;
		GraphicsAPI::DepthStencilTarget* depthTarget = nullptr;

		GraphicsAPI::UniformBuffer* shadowMapUniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;
		*/

		REFLECT("PointLight")
	};

	void SetupPointLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroyPointLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
