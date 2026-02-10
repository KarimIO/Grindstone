#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include "GpuQueue.hpp"

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>

#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone::GraphicsAPI {
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	struct PassNode {};

	class RenderGraph {
	public:

		struct RenderGraphContext {
			Grindstone::Math::Extent2D swapchainSize;
			Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
			Grindstone::WorldContextSet* worldContextSet = nullptr;
		};

		using ResourceIndex = int32_t;

		enum class ResourceType {
			Buffer,
			Image
		};

		enum class ImageSizeType {
			Absolute,
			SwapchainRelative
		};

		struct ImageResource {
			ImageSizeType sizeClass = ImageSizeType::SwapchainRelative;
			float width = 1.0f;
			float height = 1.0f;
			uint32_t samples = 1;
			uint32_t mipLevels = 1;
			uint32_t depth = 1;
			Grindstone::GraphicsAPI::Format format;
		};

		struct BufferResource {
			uint64_t size;
			Grindstone::GraphicsAPI::BufferUsage bufferUsage;
		};

		struct PassImageAttachmentWrite {
			
		};

		struct PassImageAttachmentRead {
		};

		struct PassImageAttachmentReadWrite {
		};

		struct PassBufferCreationInfo {};

		class RenderPass {
		public:
			void ReadStorageImage(Grindstone::HashedString inputName, ImageResource resource);
			void ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageResource resource);
			void WriteStorageImage(Grindstone::HashedString outputName, ImageResource resource);

			void ReadColorAttachment(Grindstone::HashedString inputName, ImageResource resource);
			void ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageResource resource);
			void WriteColorAttachment(Grindstone::HashedString outputName, ImageResource resource, Grindstone::GraphicsAPI::ClearColor clearValue);

			void ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageResource resource);
			void ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageResource resource);
			void WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageResource resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

			void ReadBuffer(Grindstone::HashedString inputname, BufferResource resource);
			void WriteBuffer(Grindstone::HashedString outputName, BufferResource resource);
			void ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferResource resource);

			GpuQueue type;

			std::vector<PassImageRead> imageReads;
			std::vector<PassImageWrite> imageWrites;

			std::vector<PassImageRead> colorAttachments;

			std::vector<PassBufferCreationInfo> bufferCreations;
			std::vector<const char*> bufferDependencies;
		};

		class RenderPassExecution {

		};

		void Print();

		void AddComputePass(const std::string_view name, std::function<void(RenderPass&)> setup, std::function<void(RenderGraphContext&, RenderPassExecution&)> exec);
		void AddGraphicsPass(const std::string_view name, std::function<void(RenderPass&)> setup, std::function<void(RenderGraphContext&, RenderPassExecution&)> exec);

		void ExecuteGraph(Grindstone::Renderer::RenderGraph::RenderGraphContext context);

	private:
	};
}
