#pragma once

#include "DescriptorSetLayout.hpp"
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class Sampler;
	class Image;
	class Buffer;

	/*! A descriptor is a reference to a data to be passed to a Pipeline. A
		DescriptorSet is a list of descriptors that can be passed to the GPU together
		to more efficiently bind data.
	*/
	class DescriptorSet {
	public:
		struct Binding {
			void* itemPtr = nullptr;
			BindingType bindingType = BindingType::None;
			uint32_t count = 1;

			Binding() = default;
			Binding(const Binding& binding) = default;
			Binding(Binding&& binding) = default;
			Binding& operator=(const Binding& binding) = default;
			Binding(void* itemPtr, BindingType bindingType, uint32_t count = 1) : itemPtr(itemPtr), bindingType(bindingType), count(count) {}

			static Binding Sampler(GraphicsAPI::Sampler* samplerPtr, uint32_t count = 1) {
				return Binding(samplerPtr, BindingType::Sampler, count);
			}

			static Binding CombinedImageSampler(std::pair<Image*, GraphicsAPI::Sampler*>* combinedSamplerPtr, uint32_t count = 1) {
				return Binding(combinedSamplerPtr, BindingType::CombinedImageSampler, count);
			}

			static Binding SampledImage(GraphicsAPI::Image* imagePtr, uint32_t count = 1) {
				return Binding(imagePtr, BindingType::SampledImage, count);
			}

			static Binding StorageImage(GraphicsAPI::Image* imagePtr, uint32_t count = 1) {
				return Binding(imagePtr, BindingType::StorageImage, count);
			}

			static Binding UniformTexelBuffer(GraphicsAPI::Image* imagePtr, uint32_t count = 1) {
				return Binding(imagePtr, BindingType::UniformTexelBuffer, count);
			}

			static Binding StorageTexelBuffer(GraphicsAPI::Image* imagePtr, uint32_t count = 1) {
				return Binding(imagePtr, BindingType::StorageTexelBuffer, count);
			}

			static Binding UniformBuffer(GraphicsAPI::Buffer* bufferPtr, uint32_t count = 1) {
				return Binding(bufferPtr, BindingType::UniformBuffer, count);
			}

			static Binding StorageBuffer(GraphicsAPI::Buffer* bufferPtr, uint32_t count = 1) {
				return Binding(bufferPtr, BindingType::StorageBuffer, count);
			}

			static Binding UniformBufferDynamic(GraphicsAPI::Buffer* bufferPtr, uint32_t count = 1) {
				return Binding(bufferPtr, BindingType::UniformBufferDynamic, count);
			}

			static Binding StorageBufferDynamic(GraphicsAPI::Buffer* bufferPtr, uint32_t count = 1) {
				return Binding(bufferPtr, BindingType::UniformBufferDynamic, count);
			}

		};

		struct CreateInfo {
			const char* debugName = nullptr;
			DescriptorSetLayout* layout = nullptr;
			const Binding* bindings = nullptr;
			uint32_t bindingCount = 0;
		};
	public:
		virtual void ChangeBindings(const DescriptorSet::Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) = 0;
	};
}
