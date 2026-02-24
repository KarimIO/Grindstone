#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include <Common/Rect.hpp>
#include "GpuQueue.hpp"

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class DescriptorSet;
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	enum class ImageSizeType {
		Absolute,
		SwapchainRelative
	};

	struct ImageDescription {
		ImageSizeType sizeClass = ImageSizeType::SwapchainRelative;
		float width = 1.0f;
		float height = 1.0f;
		uint32_t samples = 1;
		uint32_t mipLevels = 1;
		uint32_t depth = 1;
		uint32_t arrayLayers = 1;
		Grindstone::GraphicsAPI::Format format;

		GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;

		bool operator==(const ImageDescription& other) const {
			return sizeClass == other.sizeClass &&
				width == other.width &&
				height == other.height &&
				samples == other.samples &&
				mipLevels == other.mipLevels &&
				depth == other.depth &&
				format == other.format;
		}
	};

	struct TransientImageDescription {
		uint32_t width = 1u;
		uint32_t height = 1u;
		uint32_t samples = 1u;
		uint32_t mipLevels = 1u;
		uint32_t depth = 1u;
		uint32_t arrayLayers = 1u;
		Grindstone::GraphicsAPI::Format format;

		GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;

		bool operator==(const TransientImageDescription& other) const {
			return width == other.width &&
				height == other.height &&
				samples == other.samples &&
				mipLevels == other.mipLevels &&
				depth == other.depth &&
				arrayLayers == other.arrayLayers &&
				format == other.format &&
				imageDimensions == other.imageDimensions &&
				memoryUsage == other.memoryUsage &&
				imageUsage == other.imageUsage;
		}
	};

	struct BufferDescription {
		uint64_t size;
		Grindstone::GraphicsAPI::BufferUsage bufferUsage;
		Grindstone::GraphicsAPI::MemoryUsage memoryUsage;

		BufferDescription() = default;
		BufferDescription(const BufferDescription& other) = default;
		BufferDescription(BufferDescription&& other) noexcept = default;
		BufferDescription& operator=(const BufferDescription& other) = default;
		BufferDescription& operator=(BufferDescription&& other) noexcept = default;

		bool operator==(const BufferDescription& other) const {
			return size == other.size &&
				bufferUsage == other.bufferUsage;
		}
	};
}

namespace std {
	template<>
	struct std::hash<Grindstone::Renderer::BufferDescription> {
		std::size_t operator()(const Grindstone::Renderer::BufferDescription& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.bufferUsage) |
				static_cast<size_t>(desc.memoryUsage) << 32
				);
			result ^= std::hash<size_t>{}(static_cast<size_t>(desc.size));
			return result;
		}
	};

	template<>
	struct std::hash<Grindstone::Renderer::TransientImageDescription> {
		std::size_t operator()(const Grindstone::Renderer::TransientImageDescription& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.width) |
				static_cast<size_t>(desc.height) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.samples) |
				static_cast<size_t>(desc.mipLevels) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.depth) |
				static_cast<size_t>(desc.arrayLayers) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.format) |
				static_cast<size_t>(desc.imageDimensions) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.memoryUsage) |
				static_cast<size_t>(desc.imageUsage.GetValueUnderlying()) << 32
				);

			return result;
		}
	};
}

namespace Grindstone::Renderer {
	struct TransientImageData {
		GraphicsAPI::Image* image;
		GraphicsAPI::ImageLayout currentLayout;
		GraphicsAPI::AccessFlags currentAccessFlags;
	};

	struct TransientBufferData {
		GraphicsAPI::Buffer* buffer;
		GraphicsAPI::AccessFlags currentAccessFlags;
	};

	union TransientResourceUnion {
		TransientImageData image;
		TransientBufferData buffer;
	};

	class TransientResourceManager {
	public:

		void StartFrame();

		std::tuple<TransientImageData, size_t> AddTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription desc);
		std::tuple<TransientBufferData, size_t> AddTrackedBuffer(BufferDescription desc);

		Grindstone::Renderer::TransientImageData& GetTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription inDesc, size_t index);
		Grindstone::Renderer::TransientBufferData& GetTrackedBuffer(BufferDescription desc, size_t index);

	protected:

		struct TransientImage {
			bool isUsedThisFrame = false;
			int8_t lifetime;
			TransientImageData data;
		};

		struct TransientBuffer {
			bool isUsedThisFrame = false;
			int8_t lifetime;
			TransientBufferData data;
		};
		std::unordered_map<TransientImageDescription, std::vector<TransientImage>> images;
		std::unordered_map<BufferDescription, std::vector<TransientBuffer>> buffers;

	};

	struct PassNode {};

	class RenderGraph {
	public:

		struct RenderGraphContext {
			Grindstone::GraphicsAPI::DescriptorSet* globalDescriptorSet = nullptr;
			Grindstone::Math::Extent2D swapchainSize;
			Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
			Grindstone::WorldContextSet* worldContextSet = nullptr;
			uint32_t swapchainIndex = 0;
		};

		using ResourceIndex = int32_t;

		enum class ResourceType {
			UniformBuffer,
			StorageBuffer,
			ColorAttachment,
			DepthAttachment,
			StorageImage
		};

		struct UnionResource {
			ResourceType resourceType;
			union {
				BufferDescription buffer;
				ImageDescription image;
			};

			UnionResource() {}
			UnionResource(const UnionResource& other) : resourceType(other.resourceType) {
				switch (resourceType) {
				case ResourceType::ColorAttachment:
				case ResourceType::DepthAttachment:
				case ResourceType::StorageImage:
					image = other.image;
					break;
				case ResourceType::StorageBuffer:
				case ResourceType::UniformBuffer:
					buffer = other.buffer;
					break;
				default:
					GS_ASSERT("Invalid Resource Type!");
				}
			}
			UnionResource(BufferDescription buffer) : resourceType(((buffer.bufferUsage & GraphicsAPI::BufferUsage::Uniform) == GraphicsAPI::BufferUsage::Uniform) ? ResourceType::UniformBuffer : ResourceType::StorageBuffer), buffer(buffer) {}
			UnionResource(ResourceType resourceType, ImageDescription image) : resourceType(resourceType), image(image) {}
			UnionResource(UnionResource&& other) noexcept = default;
			UnionResource& operator=(const UnionResource& other) {
				resourceType = other.resourceType;
				switch (resourceType) {
				case ResourceType::ColorAttachment:
				case ResourceType::DepthAttachment:
				case ResourceType::StorageImage:
					image = other.image;
					break;
				case ResourceType::StorageBuffer:
				case ResourceType::UniformBuffer:
					buffer = other.buffer;
					break;
				default:
					GS_ASSERT("Invalid Resource Type!");
				}

				return *this;
			}
			UnionResource& operator=(UnionResource&& other) noexcept = default;


			bool operator==(const UnionResource& other) const {
				if (resourceType != other.resourceType) {
					return false;
				}

				switch (resourceType) {
				case ResourceType::ColorAttachment:
				case ResourceType::DepthAttachment:
				case ResourceType::StorageImage:
					return image == other.image;
				case ResourceType::UniformBuffer:
				case ResourceType::StorageBuffer:
					return buffer == other.buffer;
				default:
					GS_ASSERT("Invalid Resource Type!");
				}

				return false;
			}
		};

		struct ResourceRead {
			Grindstone::HashedString name;
			UnionResource resource;

			ResourceRead() = default;
			ResourceRead(Grindstone::HashedString name, UnionResource resource) : name(name), resource(resource) {}
			ResourceRead(const ResourceRead& other) = default;
			ResourceRead(ResourceRead&& other) noexcept = default;
			ResourceRead& operator=(const ResourceRead& other) = default;
			ResourceRead& operator=(ResourceRead&& other) noexcept = default;

			bool operator==(const ResourceRead& other) {
				return name == other.name &&
					resource == other.resource;
			}
		};

		struct ResourceWrite {
			Grindstone::HashedString name;
			UnionResource resource;

			ResourceWrite() = default;
			ResourceWrite(
				Grindstone::HashedString name,
				UnionResource resource
			) : name(name), resource(resource) {
			}
			ResourceWrite(const ResourceWrite& other) = default;
			ResourceWrite(ResourceWrite&& other) noexcept = default;
			ResourceWrite& operator=(const ResourceWrite& other) = default;
			ResourceWrite& operator=(ResourceWrite&& other) noexcept = default;

			bool operator==(const ResourceWrite& other) {
				return name == other.name &&
					resource == other.resource;
			}
		};

		struct ResourceReadWrite {
			Grindstone::HashedString input;
			Grindstone::HashedString output;
			UnionResource resource;

			ResourceReadWrite() = default;
			ResourceReadWrite(
				Grindstone::HashedString input,
				Grindstone::HashedString output,
				UnionResource resource
			) : input(input), output(output), resource(resource) {
			}
			ResourceReadWrite(const ResourceReadWrite& other) = default;
			ResourceReadWrite(ResourceReadWrite&& other) noexcept = default;
			ResourceReadWrite& operator=(const ResourceReadWrite& other) = default;
			ResourceReadWrite& operator=(ResourceReadWrite&& other) noexcept = default;

			bool operator==(const ResourceReadWrite& other) {
				return input == other.input &&
					output == other.output &&
					resource == other.resource;
			}
		};

		struct PassBufferCreationInfo {};

		using ResourceId = size_t;

		class RenderPassExecution {
			Grindstone::GraphicsAPI::DescriptorSet* GetPassDescriptorSet() const;
		public:
			Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		};

		class RenderPass {
		public:
			void ReadStorageImage(Grindstone::HashedString inputName, ImageDescription resource);
			void ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
			void WriteStorageImage(Grindstone::HashedString outputName, ImageDescription resource);

			void ReadColorAttachment(Grindstone::HashedString inputName, ImageDescription resource);
			void ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
			void WriteColorAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue);

			void ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageDescription resource);
			void ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
			void WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

			void ReadBuffer(Grindstone::HashedString inputname, BufferDescription resource);
			void WriteBuffer(Grindstone::HashedString outputName, BufferDescription resource);
			void ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferDescription resource);

			Grindstone::HashedString name;

			GpuQueue type;

			std::vector<ResourceWrite> writes;
			std::vector<ResourceRead> reads;
			std::vector<ResourceReadWrite> readWrites;

			std::function<void(RenderGraphContext&, Grindstone::Renderer::RenderGraph::RenderPassExecution&)> execution;
		};

		void AddComputePass(Grindstone::HashedString name, std::function<void(RenderPass&)> setup, std::function<void(RenderGraphContext&, RenderPassExecution&)> exec);
		void AddGraphicsPass(Grindstone::HashedString name, std::function<void(RenderPass&)> setup, std::function<void(RenderGraphContext&, RenderPassExecution&)> exec);

		void ExecuteGraph(Grindstone::Renderer::RenderGraph::RenderGraphContext context);

	protected:

		void SortPasses();
		void SetupDependencies(
			std::vector<ResourceWrite>& resources,
			std::map<Grindstone::HashedString, ResourceId>& namesToResourceIds
		);

		std::vector<Grindstone::Renderer::RenderGraph::RenderPass> passes;
		Grindstone::Renderer::TransientResourceManager transientResourceManager;

	};
}
