#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class Buffer;
		class DescriptorSet;
		class DescriptorSetLayout;
		class RenderPass;
		class Framebuffer;
		class Image;
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
		float sourceRadius = 0.0f;
		float intensity = 0.0f;
		uint32_t shadowResolution = 0u;
		uint32_t cachedShadowResolution = 0u;

		GraphicsAPI::RenderPass* renderPass = nullptr;
		GraphicsAPI::Framebuffer* framebuffer = nullptr;
		GraphicsAPI::Image* depthTarget = nullptr;

		GraphicsAPI::Buffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::Buffer* shadowMapUniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("DirectionalLight")
	};

	void SetupDirectionalLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroyDirectionalLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
