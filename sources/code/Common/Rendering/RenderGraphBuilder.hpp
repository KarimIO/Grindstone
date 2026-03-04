#pragma once

#include <vector>

#include <Common/HashedString.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include "RenderGraphBuilderPass.hpp"
#include "RenderGraph.hpp"

namespace Grindstone::Renderer {
	using PassId = size_t;
	using ResourceId = size_t;
	const size_t invalidResourceId = std::numeric_limits<size_t>().max();

	class RenderGraphBuilder {
	public:
		template<typename ReturnType>
		ReturnType CreateGraphicsPass(
			Grindstone::StringRef name,
			std::function<ReturnType(Grindstone::Renderer::GraphicsRenderGraphBuilderPass<ReturnType>&)> setupImmediateCallback,
			std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&)> executionCallback
		) {
			auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<GraphicsRenderGraphBuilderPass<ReturnType>>());
			auto pass = static_cast<GraphicsRenderGraphBuilderPass<ReturnType>*>(uniquePtr.Get());
			pass->name = name;
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
			pass->SetExecutionCallback(executionCallback);
			return setupImmediateCallback(*pass);
		}

		TransferRenderGraphBuilderPass* CreateTransferPass(
			Grindstone::StringRef name,
			std::function<void(Grindstone::Renderer::TransferRenderGraphBuilderPass&)> setupImmediateCallback
		);

		PresentRenderGraphBuilderPass* CreatePresentPass(TGBImageRef imageRef);
		void CreatePresentPass(
			std::function<void(Grindstone::Renderer::PresentRenderGraphBuilderPass&)> setupImmediateCallback
		);

		Grindstone::Renderer::RenderGraph Compile();

	protected:

		std::vector<Grindstone::UniquePtr<RenderGraphBuilderPass>> passes;
		std::vector<ImageDescription> images;
		std::vector<BufferDescription> buffers;
		ResourceId presentationResourceId = invalidResourceId;

	};
}
