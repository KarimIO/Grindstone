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
		static constexpr size_t FRAME_COUNT = 3;
		static constexpr size_t MAX_CASCADE_COUNT = 8;
		struct ShadowData {
			Grindstone::Math::Matrix4 shadowMatrix;
			Grindstone::Math::Rect2D shadowRenderArea;
		};

		struct UniformStruct {
			ShadowData shadowData[MAX_CASCADE_COUNT];
			float cascadeDistances[MAX_CASCADE_COUNT];
			Grindstone::Math::Matrix4 cameraViewMatrix;
			Math::Float3 color;
			uint32_t cascadeCount;
			Math::Float3 direction;
			float padding0; // padded for shaders.
			Math::Float3 eyePosForShadowing;
		};

		static void Construct(Grindstone::WorldContextSet& cxtSet, entt::entity);
		static void Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity);

		glm::vec3 eyePosForShadowing;
		Grindstone::Math::Matrix4 debugCameraProjectionMatrix;
		Grindstone::Math::Matrix4 debugCameraViewMatrix;
		Grindstone::Math::Matrix4 debugShadowProjectionMatrix[8];
		Grindstone::Math::Matrix4 shadowProjectionMatrix[8];
		Grindstone::Math::Matrix4 shadowViewMatrix[8];
		Grindstone::Math::Matrix4 shadowProjViewMatrix[8];
		Grindstone::Math::Rect2D shadowRenderArea[8];
		float cascadeDistances[8];
		uint32_t cascadeCount = 4;
		Math::Float3 color = Math::Float3(1.0f, 1.0f, 1.0f);
		float intensity = 100.0f;

		GraphicsAPI::Buffer* uniformBufferObject[FRAME_COUNT];
		GraphicsAPI::DescriptorSet* descriptorSet[FRAME_COUNT];
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		GraphicsAPI::Buffer* shadowMapUniformBufferObjects[FRAME_COUNT][MAX_CASCADE_COUNT];
		GraphicsAPI::DescriptorSet* shadowMapDescriptorSets[FRAME_COUNT][MAX_CASCADE_COUNT];
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		REFLECT("DirectionalLight")
	};
}
