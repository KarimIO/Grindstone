#pragma once

#include <vector>

#include <Common/HashedString.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include "RenderGraphBuilderPass.hpp"
#include "RenderGraph.hpp"

namespace Grindstone::Renderer {
	class RenderGraphBuilder {
	public:
		template<typename ReturnType, typename SetupCallback, typename ExecutionCallback>
		ReturnType CreateGraphicsPass(
			Grindstone::StringRef name,
			Grindstone::Renderer::MetaRect metaRect,
			SetupCallback setupImmediateCallback,
			ExecutionCallback executionCallback
		) {
			static_assert(std::is_invocable_r_v<ReturnType, SetupCallback, Grindstone::Renderer::GraphicsRenderGraphBuilderPass<ReturnType>&>, "Rendergraph setup callback must match expected signature.");
			static_assert(std::is_invocable_r_v<void, ExecutionCallback, Grindstone::Math::IntRect2D, const Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&>, "Rendergraph execution callback must match expected signature.");
			auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<GraphicsRenderGraphBuilderPass<ReturnType>>());
			auto pass = static_cast<GraphicsRenderGraphBuilderPass<ReturnType>*>(uniquePtr.Get());
			pass->name = name;
			pass->renderGraphBuilder = this;
			pass->SetExecutionCallback(executionCallback);
			return setupImmediateCallback(*pass);
		}

		template<typename ReturnType>
		ReturnType CreateComputePass(
			Grindstone::StringRef name,
			std::function<ReturnType(Grindstone::Renderer::ComputeRenderGraphBuilderPass<ReturnType>&)> setupImmediateCallback,
			std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::ComputeRenderGraphPass<ReturnType>&, ReturnType&)> executionCallback
		) {
			auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<ComputeRenderGraphBuilderPass<ReturnType>>());
			auto pass = static_cast<ComputeRenderGraphBuilderPass<ReturnType>*>(uniquePtr.Get());
			pass->name = name;
			pass->renderGraphBuilder = this;
			pass->SetExecutionCallback(executionCallback);
			return setupImmediateCallback(*pass);
		}

		TransferRenderGraphBuilderPass* CreateTransferPass(
			Grindstone::StringRef name,
			std::function<void(Grindstone::Renderer::TransferRenderGraphBuilderPass&)> setupImmediateCallback
		);

		PresentRenderGraphBuilderPass* CreatePresentPass(RenderGraphBuilderResourceRef imageRef);
		void CreatePresentPass(
			std::function<void(Grindstone::Renderer::PresentRenderGraphBuilderPass&)> setupImmediateCallback
		);

		Grindstone::Renderer::RenderGraphBuilderResourceRef AddImage(ImageDescription imageDesc, Renderer::PassId passId = Renderer::invalidPassId);
		Grindstone::Renderer::RenderGraphBuilderResourceRef AddBuffer(BufferDescription bufferDesc, Renderer::PassId passId = Renderer::invalidPassId);

		Grindstone::Renderer::RenderGraph Compile() const;

		void Clear();

		using GetExternalImageCallback = std::function<Grindstone::GraphicsAPI::Image*>;
		using GetExternalBufferCallback = std::function<Grindstone::GraphicsAPI::Buffer*>;

	protected:

		std::vector<Grindstone::UniquePtr<RenderGraphBuilderPass>> passes;
		std::vector<ImageDescription> images;
		std::vector<BufferDescription> buffers;
		ResourceId presentationResourceId = invalidResourceId;

	};
}
