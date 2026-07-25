#include <glm/glm.hpp>

#include <Common/Graphics/Core.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/SkinningPass.hpp>
#include <Grindstone.Renderables.3D/include/Components/AnimatorComponent.hpp>
#include <Grindstone.Renderables.3D/include/Components/SkeletalMeshComponent.hpp>
#include <Grindstone.Renderables.3D/include/Assets/Mesh3dImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Renderer;

enum class SkeletalMeshLayoutIndex {
	Position = 0,
	Normal,
	Tangent,
	Uv0,
	Uv1
};

static Grindstone::GraphicsAPI::VertexInputLayout PrepareLayouts() {
	GraphicsAPI::VertexInputLayoutBuilder builder;
	builder.AddBinding(
		{ 0, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexPosition",
				(uint32_t)SkeletalMeshLayoutIndex::Position,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		}
	).AddBinding(
		{ 1, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexNormal",
				(uint32_t)SkeletalMeshLayoutIndex::Normal,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Normal
			}
		}
	).AddBinding(
		{ 2, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexTangent",
				(uint32_t)SkeletalMeshLayoutIndex::Tangent,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Tangent
			}
		}
	).AddBinding(
		{ 3, 2 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexTexCoord0",
				(uint32_t)SkeletalMeshLayoutIndex::Uv0,
				Grindstone::GraphicsAPI::Format::R32G32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
			}
		}
	);

	return builder.Build();
}

bool Grindstone::Renderer::SkinningPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	skinningPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/skinning");

	return true;
}

void Grindstone::Renderer::SkinningPass::AddPass(
	Renderer::RenderGraphBuilder& renderGraphBuilder,
	WorldContextSet& worldContextSet
) {
	ComputePipelineAsset* skinningPipelineAsset = skinningPipelineSet.Get();
	if (skinningPipelineAsset == nullptr) {
		return;
	}

	GraphicsAPI::ComputePipeline* skinningPipeline = skinningPipelineAsset->GetPipeline();
	if (skinningPipeline == nullptr) {
		return;
	}

	GraphicsAPI::PipelineLayout* pipelineLayout = skinningPipelineAsset->GetPipelineLayout();

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	registry.view<TagComponent, AnimatorComponent, SkeletalMeshComponent>().each(
		[skinningPipeline, &renderGraphBuilder, &worldContextSet, pipelineLayout](
			TagComponent& tagComp,
			AnimatorComponent& animatorComp,
			SkeletalMeshComponent& skeletalMeshComp
		) {
			Mesh3dAsset* meshAsset = skeletalMeshComp.mesh.Get();

			if (meshAsset == nullptr) {
				return;
			}

			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			uint32_t vertexCount = meshAsset->vertexCount;
			if (skeletalMeshComp.vertexCountBuffer == nullptr) {
				std::string vertexCountBufferName = std::vformat("Skinning VertexCount Buffer '{}'", std::make_format_args(tagComp.tag));
				GraphicsAPI::Buffer::CreateInfo vertexCountBufferCreateInfo{
					.debugName = vertexCountBufferName.c_str(),
					.content = &vertexCount,
					.bufferSize = sizeof(uint32_t),
					.bufferUsage = GraphicsAPI::BufferUsage::Uniform |
						GraphicsAPI::BufferUsage::TransferSrc |
						GraphicsAPI::BufferUsage::TransferDst,
					.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU
				};
				skeletalMeshComp.vertexCountBuffer = graphicsCore->CreateBuffer(vertexCountBufferCreateInfo);
			}

			if (skeletalMeshComp.vertexPositionBuffer == nullptr) {
				std::string vertexPositionBufferName = std::vformat("Skinned Vertex Pos Buffer '{}'", std::make_format_args(tagComp.tag));
				GraphicsAPI::Buffer::CreateInfo vertexPosBufferCreateInfo{
					.debugName = vertexPositionBufferName.c_str(),
					.content = nullptr,
					.bufferSize = meshAsset->positionBuffer->GetSize(),
					.bufferUsage = GraphicsAPI::BufferUsage::Vertex |
						GraphicsAPI::BufferUsage::Storage |
						GraphicsAPI::BufferUsage::TransferSrc |
						GraphicsAPI::BufferUsage::TransferDst,
					.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly
				};
				skeletalMeshComp.vertexPositionBuffer = graphicsCore->CreateBuffer(vertexPosBufferCreateInfo);
			}

			if (skeletalMeshComp.vertexNormalBuffer == nullptr) {
				std::string vertexNormalBufferName = std::vformat("Skinned Vertex Normal Buffer '{}'", std::make_format_args(tagComp.tag));
				GraphicsAPI::Buffer::CreateInfo vertexNormalBufferCreateInfo{
					.debugName = vertexNormalBufferName.c_str(),
					.content = nullptr,
					.bufferSize = meshAsset->normalBuffer->GetSize(),
					.bufferUsage = GraphicsAPI::BufferUsage::Vertex |
						GraphicsAPI::BufferUsage::Storage |
						GraphicsAPI::BufferUsage::TransferSrc |
						GraphicsAPI::BufferUsage::TransferDst,
					.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly
				};
				skeletalMeshComp.vertexNormalBuffer = graphicsCore->CreateBuffer(vertexNormalBufferCreateInfo);
			}

			if (skeletalMeshComp.vertexTangentBuffer == nullptr) {
				std::string vertexTangentBufferName = std::vformat("Skinned Vertex Tangent Buffer '{}'", std::make_format_args(tagComp.tag));
				GraphicsAPI::Buffer::CreateInfo vertexTangentBufferCreateInfo{
					.debugName = vertexTangentBufferName.c_str(),
					.content = nullptr,
					.bufferSize = meshAsset->tangentBuffer->GetSize(),
					.bufferUsage = GraphicsAPI::BufferUsage::Vertex |
						GraphicsAPI::BufferUsage::Storage |
						GraphicsAPI::BufferUsage::TransferSrc |
						GraphicsAPI::BufferUsage::TransferDst,
					.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly
				};
				skeletalMeshComp.vertexTangentBuffer = graphicsCore->CreateBuffer(vertexTangentBufferCreateInfo);
			}

			if (skeletalMeshComp.skinnedVertexArrayObject == nullptr) {
				GraphicsAPI::VertexInputLayout inputLayout = PrepareLayouts();

				std::array<GraphicsAPI::Buffer*, 4> vertexBuffers{
					skeletalMeshComp.vertexPositionBuffer,
					skeletalMeshComp.vertexNormalBuffer,
					skeletalMeshComp.vertexTangentBuffer,
					meshAsset->uvBuffers[0]
				};

				GraphicsAPI::VertexArrayObject::CreateInfo vaoCreateInfo{
					.debugName = "Skinned Vertex Layout",
					.vertexBuffers = vertexBuffers.data(),
					.vertexBufferCount = static_cast<uint32_t>(vertexBuffers.size()),
					.indexBuffer = meshAsset->indexBuffer,
					.layout = inputLayout
				};
				skeletalMeshComp.skinnedVertexArrayObject = graphicsCore->CreateVertexArrayObject(vaoCreateInfo);
			}

			if (skeletalMeshComp.skinningDescriptorSet == nullptr) {
				std::string descriptorSetName = std::vformat("Skinning Descriptor Set '{}'", std::make_format_args(tagComp.tag));
				std::string descriptorSetLayoutName = std::vformat("Skinning Descriptor Set Layout '{}'", std::make_format_args(tagComp.tag));
				std::array<GraphicsAPI::DescriptorSet::Binding, 10> bindings{
					GraphicsAPI::DescriptorSet::Binding::UniformBuffer(skeletalMeshComp.vertexCountBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(animatorComp.skeletonMatrixBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(meshAsset->positionBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(meshAsset->normalBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(meshAsset->tangentBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(meshAsset->boneIdsBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(meshAsset->boneWeightsBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(skeletalMeshComp.vertexPositionBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(skeletalMeshComp.vertexNormalBuffer),
					GraphicsAPI::DescriptorSet::Binding::StorageBuffer(skeletalMeshComp.vertexTangentBuffer),
				};

				std::array<GraphicsAPI::DescriptorSetLayout::Binding, 10> layoutBindings{
					GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 2, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 3, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 4, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 5, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 6, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 7, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 8, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
					GraphicsAPI::DescriptorSetLayout::Binding{ 9, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Compute },
				};

				GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCreateInfo{
					.debugName = descriptorSetLayoutName.c_str(),
					.bindings = layoutBindings.data(),
					.bindingCount = static_cast<uint32_t>(layoutBindings.size())
				};
				GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(layoutCreateInfo);

				GraphicsAPI::DescriptorSet::CreateInfo createInfo{
					.debugName = descriptorSetName.c_str(),
					.layout = descriptorSetLayout,
					.bindings = bindings.data(),
					.bindingCount = static_cast<uint32_t>(bindings.size())
				};
				skeletalMeshComp.skinningDescriptorSet = graphicsCore->CreateDescriptorSet(createInfo);
			}

			std::string computePassLabel = std::vformat("Skinning Pass for '{}'", std::make_format_args(tagComp.tag));
			renderGraphBuilder.CreateComputePass<RenderGraphBuilderResourceRef>(
				computePassLabel,
				[&animatorComp, &skeletalMeshComp](
					ComputeRenderGraphBuilderPass<RenderGraphBuilderResourceRef>& pass
				) -> RenderGraphBuilderResourceRef {
					RenderGraphBuilderResourceRef output{};

					return output;
				},
				[vertexCount, &skeletalMeshComp, skinningPipeline, pipelineLayout](
					RenderGraphContext& cxt,
					const RenderGraphFrameResources& frameResources,
					Grindstone::Renderer::ComputeRenderGraphPass<RenderGraphBuilderResourceRef>& pass,
					RenderGraphBuilderResourceRef& ref
				) {
					GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
					cmd->BindComputeDescriptorSet(pipelineLayout, &skeletalMeshComp.skinningDescriptorSet, 0u, 1u);
					cmd->BindComputePipeline(skinningPipeline);
					cmd->DispatchCompute((vertexCount + 63) / 64, 1, 1);
				}
			);
		}
	);
}
