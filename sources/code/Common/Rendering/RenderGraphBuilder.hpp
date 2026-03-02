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
		GraphicsRenderGraphBuilderPass* CreateGraphicsPass(Grindstone::HashedString hashedString);
		template<typename ReturnType>
		ReturnType CreateGraphicsPass(
			Grindstone::HashedString hashedString,
			std::function<ReturnType(Grindstone::Renderer::GraphicsRenderGraphBuilderPass&)> setupImmediateCallback,
			Grindstone::Renderer::GraphicsExecutionCallback executionCallback
		) {
			auto pass = CreateGraphicsPass(hashedString);
			pass->SetExecutionCallback(executionCallback);
			return setupImmediateCallback(*pass);
		}

		ComputeRenderGraphBuilderPass* CreateComputePass(Grindstone::HashedString hashedString);
		template<typename ReturnType>
		ReturnType CreateComputePass(
			Grindstone::HashedString hashedString,
			std::function<ReturnType(Grindstone::Renderer::ComputeRenderGraphBuilderPass&)> setupImmediateCallback,
			Grindstone::Renderer::ComputeExecutionCallback executionCallback
		) {
			auto pass = CreateGraphicsPass(hashedString);
			pass->SetExecutionCallback(executionCallback);
			return setupImmediateCallback(*pass);
		}

		TransferRenderGraphBuilderPass* CreateTransferPass(Grindstone::HashedString hashedString);
		TransferRenderGraphBuilderPass* CreateTransferPass(
			Grindstone::HashedString hashedString,
			std::function<void(Grindstone::Renderer::TransferRenderGraphBuilderPass&)> setupImmediateCallback
		);

		PresentRenderGraphBuilderPass* CreatePresentPass();
		PresentRenderGraphBuilderPass* CreatePresentPass(
			std::function<void(Grindstone::Renderer::PresentRenderGraphBuilderPass&)> setupImmediateCallback
		);

		Grindstone::Renderer::RenderGraph Compile();

	protected:

		std::unordered_map<Grindstone::HashedString, Grindstone::UniquePtr<RenderGraphBuilderPass>> passes;
		std::unordered_map<Grindstone::HashedString, ResourceId> resources;
		ResourceId presentationResourceId = invalidResourceId;

	};
}
