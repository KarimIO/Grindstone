#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Assets/PipelineSet/ComputePipelineAsset.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>

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
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		) override;

		virtual uint16_t GetRenderModeCount() const override;
		virtual const RenderMode* GetRenderModes() const override;
		virtual void SetRenderMode(uint16_t mode) override;

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
		struct DeferredRendererImageSet {
			GraphicsAPI::Framebuffer* gbuffer = nullptr;
			GraphicsAPI::Image* gbufferDepthStencilTarget = nullptr;
			GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* lightingFramebuffer = nullptr;
			GraphicsAPI::Image* litHdrRenderTarget = nullptr;
			GraphicsAPI::Image* ssrRenderTarget = nullptr;
			GraphicsAPI::Image* gbufferAlbedoRenderTarget = nullptr;
			GraphicsAPI::Image* gbufferNormalRenderTarget = nullptr;
			GraphicsAPI::Image* gbufferSpecularRoughnessRenderTarget = nullptr;

			GraphicsAPI::Framebuffer* ambientOcclusionFramebuffer = nullptr;
			GraphicsAPI::Image* ambientOcclusionRenderTarget = nullptr;

			GraphicsAPI::Buffer* globalUniformBufferObject = nullptr;
			GraphicsAPI::Buffer* debugUniformBufferObject = nullptr;
			GraphicsAPI::Buffer* tonemapPostProcessingUniformBufferObject = nullptr;

			GraphicsAPI::DescriptorSet* ssrDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* tonemapDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* lightingDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* debugDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

			GraphicsAPI::DescriptorSet* dofSourceDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofNearBlurDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofFarBlurDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofCombineDescriptorSet = nullptr;

			GraphicsAPI::DescriptorSet* ambientOcclusionDescriptorSet = nullptr;

			GraphicsAPI::Image* nearDofRenderTarget = nullptr;
			GraphicsAPI::Image* farDofRenderTarget = nullptr;
			GraphicsAPI::Image* nearBlurredDofRenderTarget = nullptr;
			GraphicsAPI::Image* farBlurredDofRenderTarget = nullptr;

			GraphicsAPI::Framebuffer* dofSeparationFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofNearBlurFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofFarBlurFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofCombinationFramebuffer = nullptr;

			std::vector<GraphicsAPI::Image*> bloomRenderTargets;
			std::vector<GraphicsAPI::DescriptorSet*> bloomDescriptorSets;
		};

		void CreatePipelines();
		void CreateBloomUniformBuffers();
		void UpdateBloomDescriptorSet(DeferredRendererImageSet& imageSet);
		void CreateDepthOfFieldRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void CreateSsrRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void CreateBloomRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void RenderDepthOfField(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void RenderSsr(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void UpdateBloomUBO();

		void RenderSsao(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* commandBuffer);
		void RenderShadowMaps(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry);
		void RenderLights(uint32_t imageIndex, GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry);
		void PostProcess(uint32_t imageIndex, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void Debug(uint32_t imageIndex, GraphicsAPI::Framebuffer* outputFramebuffer, GraphicsAPI::CommandBuffer* commandBuffer);

		void CreateDepthOfFieldResources();
		void CreateBloomResources();
		void CreateSSRResources();
		void CreateSsaoKernelAndNoise();
		void CreateDescriptorSetLayouts();
		void CreateGbufferFramebuffer();
		void CreateLitHDRFramebuffer();
		void CreateDescriptorSets(DeferredRendererImageSet& imageSet);
		void UpdateDescriptorSets(DeferredRendererImageSet& imageSet);
		void CreateUniformBuffers();
		void CreateVertexAndIndexBuffersAndLayouts();

		struct PostProcessUbo {
			glm::vec4 vignetteColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			float vignetteRadius = 0.75f;
			float vignetteSoftness = 0.8f;
			float grainAmount = 0.0007f;
			float grainPixelSize = 1.0f;
			glm::vec2 chromaticDistortionRedOffset = glm::vec2(0.00045f, 0.00045f);
			glm::vec2 chromaticDistortionGreenOffset = glm::vec2(0.0003f, 0.0003f);
			glm::vec2 chromaticDistortionBlueOffset = glm::vec2(-0.0003f, -0.0003f);
			float paniniDistortionStrength = 0.0f;
			bool isAnimated = true;
		};

		PostProcessUbo postProcessUboData;

		struct DebugUboData {
			uint16_t renderMode;
		};

		DebugUboData debugUboData;

		uint32_t framebufferWidth = 0u;
		uint32_t framebufferHeight = 0u;
		uint32_t renderWidth = 0u;
		uint32_t renderHeight = 0u;

		size_t bloomStoredMipLevelCount = 0;
		size_t bloomMipLevelCount = 0;
		Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut;

		std::vector<DeferredRendererImageSet> deferredRendererImageSets;
		std::vector<GraphicsAPI::Buffer*> bloomUniformBuffers;
		size_t bloomFirstUpsampleIndex = 0;

		GraphicsAPI::Buffer* ssaoUniformBuffer = nullptr;
		GraphicsAPI::Image* ssaoNoiseTexture = nullptr;
		GraphicsAPI::Sampler* ssaoNoiseSampler = nullptr;
		GraphicsAPI::DescriptorSetLayout* ambientOcclusionDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet = nullptr;

		GraphicsAPI::DescriptorSetLayout* environmentMapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* environmentMapDescriptorSet = nullptr;

		GraphicsAPI::VertexInputLayout vertexLightPositionLayout{};

		GraphicsAPI::DescriptorSetLayout* bloomDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssrDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* tonemapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* debugDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMappedLightDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingWithShadowUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* engineDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;

		GraphicsAPI::RenderPass* targetRenderPass = nullptr;

		GraphicsAPI::Buffer* vertexBuffer;
		GraphicsAPI::Buffer* indexBuffer;
		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;

		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> ssaoPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> imageBasedLightingPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> spotLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pointLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> directionalLightPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> tonemapPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofSeparationPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofBlurPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> dofCombinationPipelineSet;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> debugPipelineSet;

		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> ssrPipelineSet;
		Grindstone::AssetReference<Grindstone::ComputePipelineAsset> bloomPipelineSet;

		GraphicsAPI::DescriptorSetLayout* dofSourceDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* dofBlurDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* dofCombinationDescriptorSetLayout = nullptr;

		// Used to check when environment map changes, so we can update it
		Uuid currentEnvironmentMapUuid;

		DeferredRenderMode renderMode;
	};
}
