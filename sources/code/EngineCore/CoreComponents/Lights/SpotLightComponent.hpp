#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "Common/Math.hpp"
#include "Common/Rect.hpp"

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
			Math::Float3 position;
			float attenuationRadius;
			Math::Float3 direction;
			float innerAngle;
			Math::Float3 color;
			float outerAngle;
			Math::Rect2D shadowRenderArea;
		};

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		Math::Matrix4 shadowMatrix;
		Math::Float3 color = Math::Float3(1.0f, 0.8f, 0.6f);
		float attenuationRadius = 16.0f;
		float intensity = 16.0f;
		float innerAngle = 45.0f;
		float outerAngle = 90.0f;
		Math::Rect2D shadowRenderArea;

		GraphicsAPI::Buffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::Buffer* shadowMapUniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("SpotLight")
	};
}
