#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class DescriptorSet;
		class RenderPass;
		class DepthStencilTarget;
		class Framebuffer;
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

		GraphicsAPI::RenderPass* renderPass = nullptr;
		GraphicsAPI::Framebuffer* framebuffer = nullptr;
		GraphicsAPI::DepthStencilTarget* depthTarget = nullptr;

		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::UniformBuffer* shadowMapUniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("SpotLight")
	};

	void SetupSpotLightComponent(entt::registry&, entt::entity);
	void DestroySpotLightComponent(entt::registry&, entt::entity);
}
