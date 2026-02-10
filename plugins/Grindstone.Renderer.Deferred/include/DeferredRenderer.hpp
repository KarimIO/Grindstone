#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/BloomPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/BlurPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/DebugPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/DepthOfFieldPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ForwardPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/LightingPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ScreenSpaceAmbientOcclusionPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ScreenSpaceReflectionsPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ShadowPass.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/TonemapPass.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Buffer;
		class Framebuffer;
		class Image;
		class VertexArrayObject;
		class GraphicsPipeline;
		class ComputePipeline;
		class CommandBuffer;
	};

	class DeferredRenderer : public BaseRenderer {
	public:
		DeferredRenderer(GraphicsAPI::RenderPass* targetRenderPass);
		virtual ~DeferredRenderer();
		virtual bool OnWindowResize(Events::BaseEvent*) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Render(
			GraphicsAPI::CommandBuffer* commandBuffer,
			Grindstone::WorldContextSet& worldContextSet,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::RenderAttachment& outRenderAttachment,
			Grindstone::GraphicsAPI::Image* depthTarget
		) override;

		virtual uint16_t GetRenderModeCount() const override;
		virtual const RenderMode* GetRenderModes() const override;
		virtual void SetRenderMode(uint16_t mode) override;
		virtual std::vector<Grindstone::Rendering::GeometryRenderStats> GetRenderingStats() override;

		enum class DeferredRenderMode : uint16_t {
			Default,
			Position,
			PositionMod,
			ViewPosition,
			ViewPositionMod,
			Depth,
			DepthMod,
			Normal,
			ViewNormal,
			Albedo,
			Specular,
			Roughness,
			AmbientOcclusion,
			Count
		};

	private:
		GraphicsAPI::Buffer* gpuGlobalUniformBufferObject = nullptr;

		uint32_t framebufferWidth = 0u;
		uint32_t framebufferHeight = 0u;
		Grindstone::Math::IntRect2D renderArea;

		Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut;

		GraphicsAPI::Buffer* vertexBuffer;
		GraphicsAPI::Buffer* indexBuffer;
		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;

		DeferredRenderMode renderMode;

		Grindstone::Renderer::ShadowPass shadows;
		Grindstone::Renderer::GbufferPass gbuffer;
		Grindstone::Renderer::LightingPass lighting;
		Grindstone::Renderer::ScreenSpaceReflectionsPass ssr;
		Grindstone::Renderer::ScreenSpaceAmbientOcclusionPass ssao;
		Grindstone::Renderer::BlurPass blur;
		Grindstone::Renderer::DepthOfFieldPass dof;
		Grindstone::Renderer::BloomPass bloom;
		Grindstone::Renderer::TonemapPass tonemap;
		Grindstone::Renderer::DebugPass debug;
	};
}
