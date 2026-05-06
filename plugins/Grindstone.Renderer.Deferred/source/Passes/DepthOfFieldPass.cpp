#include <EngineCore/Assets/AssetManager.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/DepthOfFieldPass.hpp>

bool Grindstone::Renderer::DepthOfFieldPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;

	dofSeparationPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofSeparation");
	dofBlurPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofBlur");
	dofCombinationPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofCombination");

	return false;
}

void Grindstone::Renderer::DepthOfFieldPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
}

/*

void DeferredRenderer::CreateDepthOfFieldRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	EngineCore& engineCore = EngineCore::GetInstance();
	RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	if (imageSet.nearDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.nearDofRenderTarget);
	}

	if (imageSet.farDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.farDofRenderTarget);
	}

	if (imageSet.nearBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.nearBlurredDofRenderTarget);
	}

	if (imageSet.farBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.farBlurredDofRenderTarget);
	}

	if (imageSet.dofSourceDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofSourceDescriptorSet);
	}

	if (imageSet.dofNearBlurDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofNearBlurDescriptorSet);
	}

	if (imageSet.dofFarBlurDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofFarBlurDescriptorSet);
	}

	if (imageSet.dofCombineDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofCombineDescriptorSet);
	}

	{
		GraphicsAPI::Image::CreateInfo dofRtCreateInfo{};
		dofRtCreateInfo.format = GraphicsAPI::Format::R16G16B16A16_SFLOAT;
		dofRtCreateInfo.width = framebufferWidth / 2;
		dofRtCreateInfo.height = framebufferHeight / 2;
		dofRtCreateInfo.imageUsage =
			GraphicsAPI::ImageUsageFlags::RenderTarget |
			GraphicsAPI::ImageUsageFlags::Sampled;

		dofRtCreateInfo.debugName = "Near DOF Render Target";
		imageSet.nearDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Far DOF Render Target";
		imageSet.farDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Near Blurred DOF Render Target";
		imageSet.nearBlurredDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Far Blurred DOF Render Target";
		imageSet.farBlurredDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);

		imageSet.nearDofAttachment = {
			.image = imageSet.nearDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.farDofAttachment = {
			.image = imageSet.farDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.nearBlurredDofAttachment = {
			.image = imageSet.nearBlurredDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.farBlurredDofAttachment = {
			.image = imageSet.farBlurredDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> sourceDofDescriptorBindings = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferDepthStencilTarget ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget )
		};

		GraphicsAPI::DescriptorSet::CreateInfo dofSourceDescriptorSetCreateInfo{};
		dofSourceDescriptorSetCreateInfo.layout = dofSourceDescriptorSetLayout;
		dofSourceDescriptorSetCreateInfo.debugName = "Depth of Field Source Descriptor";
		dofSourceDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetCreateInfo.bindings = sourceDofDescriptorBindings.data();
		imageSet.dofSourceDescriptorSet = graphicsCore->CreateDescriptorSet(dofSourceDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding nearDofDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.nearDofRenderTarget );

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurNearDescriptorSetCreateInfo{};
		dofBlurNearDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurNearDescriptorSetCreateInfo.debugName = "Depth of Field Blur Near Descriptor";
		dofBlurNearDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurNearDescriptorSetCreateInfo.bindings = &nearDofDescriptorBinding;
		imageSet.dofNearBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurNearDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding farDofDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.farDofRenderTarget );

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurFarDescriptorSetCreateInfo{};
		dofBlurFarDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurFarDescriptorSetCreateInfo.debugName = "Depth of Field Blur Far Descriptor";
		dofBlurFarDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetCreateInfo.bindings = &farDofDescriptorBinding;
		imageSet.dofFarBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurFarDescriptorSetCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> nearAndFarDescriptorBindings = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.nearBlurredDofRenderTarget ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.farBlurredDofRenderTarget )
		};

		GraphicsAPI::DescriptorSet::CreateInfo dofCombinationDescriptorSetCreateInfo{};
		dofCombinationDescriptorSetCreateInfo.layout = dofCombinationDescriptorSetLayout;
		dofCombinationDescriptorSetCreateInfo.debugName = "Depth of Field Combination Descriptor";
		dofCombinationDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(nearAndFarDescriptorBindings.size());
		dofCombinationDescriptorSetCreateInfo.bindings = nearAndFarDescriptorBindings.data();
		imageSet.dofCombineDescriptorSet = graphicsCore->CreateDescriptorSet(dofCombinationDescriptorSetCreateInfo);
	}
}

void DeferredRenderer::CreateDepthOfFieldResources() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> sourceDofDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }, // Depth
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment } // Lit Scene
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofSourceDescriptorSetLayoutCreateInfo{};
		dofSourceDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Source Descriptor Layout";
		dofSourceDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetLayoutCreateInfo.bindings = sourceDofDescriptorBindings.data();
		dofSourceDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(dofSourceDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding farDofDescriptorBinding
			{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment };

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofBlurFarDescriptorSetLayoutCreateInfo{};
		dofBlurFarDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Blur Far Descriptor Layout";
		dofBlurFarDescriptorSetLayoutCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetLayoutCreateInfo.bindings = &farDofDescriptorBinding;
		dofBlurDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(dofBlurFarDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> nearAndFarDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }, // Near RT
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment } // Far RT
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofCombinationDescriptorSetLayoutCreateInfo{};
		dofCombinationDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Combination Descriptor Layout";
		dofCombinationDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(nearAndFarDescriptorBindings.size());
		dofCombinationDescriptorSetLayoutCreateInfo.bindings = nearAndFarDescriptorBindings.data();
		dofCombinationDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(dofCombinationDescriptorSetLayoutCreateInfo);
	}
}

void DrawGbufferPass() {
	struct GbufferData {
		Renderer::RenderGraphResource albedo;
		Renderer::RenderGraphResource normal;
		Renderer::RenderGraphResource specularRoughness;
		Renderer::RenderGraphResource depth;
	};

	Grindstone::Renderer::RenderGraphPass& gbufferPass = renderGraph.AddCallbackPass<GbufferData>(
		"Gbuffer Geometry Pass",
		[&](Renderer::RenderGraph::Builder& builder, GbufferData& data) {
			gbufferPass.AddOutputImage(attachmentNameAlbedo, attachmentAlbedo);
			gbufferPass.AddOutputImage(attachmentNameNormal, attachmentNormal);
			gbufferPass.AddOutputImage(attachmentNameSpecularRoughness, attachmentSpecularRoughness);
			gbufferPass.AddOutputImage(attachmentNameDepthStencil, attachmentDepthStencil);
		},
		[&, registry = &registry](const GbufferData& data, Renderer::RenderGraphResource& resources) {
			std::array<GraphicsAPI::RenderAttachment, 3> renderAttachments = {
				resources.Get<RenderGraphImage>(data.albedo).image,
				resources.Get<RenderGraphImage>(data.normal).image,
				resources.Get<RenderGraphImage>(data.specularRoughness).image
			};

			commandBuffer->BeginRendering(
				"Gbuffer Geometry Pass",
				renderArea,
				renderAttachments.data(),
				static_cast<uint32_t>(renderAttachments.size()),
				&resources.Get<RenderGraphImage>(data.depth).image
			);

			commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderArea.GetWidth()), static_cast<float>(renderArea.GetHeight()));
			commandBuffer->SetScissor(0, 0, renderArea.GetWidth(), renderArea.GetHeight());

			// TODO: Get Rendering Stats
			assetManager->RenderQueue(commandBuffer, renderViewData, *registry, geometryOpaqueRenderPassKey);
			commandBuffer->EndRendering();
		}
	);
}

void DeferredRenderer::RenderDepthOfField(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::ClearColor singleClearColor{ 0.0f, 0.0f, 0.0f, 0.f };

	GraphicsAPI::ClearDepthStencil depthStencilClear;

	currentCommandBuffer->BeginDebugLabelSection("Depth of Field Pass", nullptr);

	{
		std::array<Grindstone::GraphicsAPI::RenderAttachment, 2> attachments = {
			imageSet.nearDofAttachment,
			imageSet.farDofAttachment
		};
		currentCommandBuffer->BeginRendering(
			"DOF Separation Pass (Depth of Field)",
			halfRenderArea,
			attachments.data(),
			static_cast<uint32_t>(attachments.size()),
			nullptr,
			nullptr
		);

		Grindstone::GraphicsAPI::GraphicsPipeline* dofSeparationPipeline = dofSeparationPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofSourceDescriptorSet
		};
		currentCommandBuffer->BindGraphicsPipeline(dofSeparationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofSeparationPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* dofBlurPipeline = dofBlurPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);

	{
		currentCommandBuffer->BeginRendering(
			"DOF Near Blur Pass (Depth of Field)",
			quarterRenderArea,
			&imageSet.nearBlurredDofAttachment,
			1u,
			nullptr,
			nullptr
		);


		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofNearBlurDescriptorSet
		};

		currentCommandBuffer->BindGraphicsPipeline(dofBlurPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	{
		currentCommandBuffer->BeginRendering(
			"DOF Far Blur Pass (Depth of Field)",
			quarterRenderArea,
			&imageSet.farBlurredDofAttachment,
			1u,
			nullptr,
			nullptr
		);

		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofFarBlurDescriptorSet
		};

		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	{
		currentCommandBuffer->BeginRendering(
			"DOF Combination Pass (Depth of Field)",
			renderArea,
			&imageSet.litHdrAttachment,
			1u,
			nullptr,
			nullptr
		);

		std::array<GraphicsAPI::DescriptorSet*, 3> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofSourceDescriptorSet,
			imageSet.dofCombineDescriptorSet
		};
		Grindstone::GraphicsAPI::GraphicsPipeline* dofCombinationPipeline = dofCombinationPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
		currentCommandBuffer->BindGraphicsPipeline(dofCombinationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofCombinationPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	currentCommandBuffer->EndDebugLabelSection();
}
*/
