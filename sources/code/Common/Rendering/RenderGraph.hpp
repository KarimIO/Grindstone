#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include <Common/Rect.hpp>
#include "GpuPassType.hpp"

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "RenderGraphPass.hpp"

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class DescriptorSet;
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	class RenderGraph {
	public:
		using ResourceId = size_t;

		RenderGraph() = default;
		RenderGraph(std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphPass>>&& passes, const std::vector<Grindstone::Renderer::UnionResourceDescription>& resourceDescriptions);
		void ExecuteGraph(Grindstone::Renderer::RenderGraphContext context);

	protected:

		std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphPass>> passes;
		std::vector<Grindstone::Renderer::UnionResourceDescription> resourceDescriptions;

	};
}
