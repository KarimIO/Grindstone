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

		void ExecuteGraph(Grindstone::Renderer::RenderGraphContext context);

	protected:

		std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphPass>> passes;

	};
}
