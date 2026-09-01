#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Common/Rect.hpp>
#include <Common/Blackboard/Blackboard.hpp>
#include <Common/Rendering/RenderGraphResourceRef.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone::Renderer {
	struct RenderFrameViewContext {
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionViewMatrix;
		glm::mat4 inverseProjectionMatrix;
		glm::mat4 inverseViewMatrix;
		glm::mat4 inverseProjectionViewMatrix;
		glm::vec3 eyePos;
		float nearDistance;
		float farDistance;
	};

	struct RenderFrameContext {
		const static uint32_t NORMAL_VIEW = 0b1;
		const static uint32_t XR_VIEW = 0b11;
		const static uint32_t CUBEMAP_VIEW = 0b111111;

		std::vector<RenderFrameViewContext> views;
		uint32_t viewMask = NORMAL_VIEW;

		// NOTE: This assumes the same swapchain image width, height, and index for all views. I'm not sure that is the case.
		Grindstone::Math::Uint2 imageSize;
		uint8_t imageIndex = 0;
		Grindstone::Renderer::RenderGraphBuilderResourceRef colorRef = Grindstone::Renderer::RenderGraphBuilderResourceRef::Invalid();
		Grindstone::Renderer::RenderGraphBuilderResourceRef depthRef = Grindstone::Renderer::RenderGraphBuilderResourceRef::Invalid();
		Grindstone::WorldContextSet& worldContextSet;
		Grindstone::Blackboard blackboard;

		RenderFrameContext(Grindstone::WorldContextSet& cxtSet) : worldContextSet(cxtSet) {}
		RenderFrameContext(const RenderFrameContext& other) = default;
		RenderFrameContext(RenderFrameContext&& other) noexcept = default;
		RenderFrameContext& operator=(const RenderFrameContext& other) = default;
		RenderFrameContext& operator=(RenderFrameContext&& other) noexcept = default;
	};
}
