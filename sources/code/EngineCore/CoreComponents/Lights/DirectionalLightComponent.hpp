#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class DescriptorSet;
		class DescriptorSetLayout;
		class RenderPass;
		class Framebuffer;
		class DepthTarget;
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

		Math::Matrix4 shadowMatrix;
		Math::Float3 color;
		float sourceRadius;
		float intensity;
		float shadowResolution;

		GraphicsAPI::RenderPass* renderPass = nullptr;
		GraphicsAPI::Framebuffer* framebuffer = nullptr;
		GraphicsAPI::DepthTarget* depthTarget = nullptr;

		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::UniformBuffer* shadowMapUniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("DirectionalLight")
	};

	void SetupDirectionalLightComponent(entt::registry&, entt::entity);
	void DestroyDirectionalLightComponent(entt::registry&, entt::entity);
}
