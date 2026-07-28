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
		static constexpr size_t MAX_CASCADE_COUNT = 8;
		struct ShadowData {
			Grindstone::Math::Matrix4 shadowMatrix;
			Grindstone::Math::Rect2D shadowRenderArea;
		};

		struct UniformStruct {
			ShadowData shadowData[MAX_CASCADE_COUNT];
			float cascadeDistances[MAX_CASCADE_COUNT];
			Math::Float3 color;
			uint32_t cascadeCount;
			Math::Float3 direction;
		};

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		ShadowData shadowData[8];
		float cascadeDistances[8];
		uint32_t cascadeCount;
		Math::Float3 color;
		float intensity = 0.0f;

		GraphicsAPI::Buffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::Buffer* shadowMapUniformBufferObjects[MAX_CASCADE_COUNT];
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSets[MAX_CASCADE_COUNT];
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("DirectionalLight")
	};
}
