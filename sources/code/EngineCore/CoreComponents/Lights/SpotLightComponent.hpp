#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class DescriptorSet;
		class RenderPass;
		class Image;
		class Framebuffer;
		class Buffer;
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
		float attenuationRadius = 0.0f;
		float intensity = 0.0f;
		float innerAngle = 0.0f;
		float outerAngle = 0.0f;
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

		REFLECT("SpotLight")
	};

	void SetupSpotLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
	void DestroySpotLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity);
}
