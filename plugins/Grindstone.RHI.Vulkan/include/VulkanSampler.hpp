#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include <Common/Graphics/Sampler.hpp>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Sampler : public Grindstone::GraphicsAPI::Sampler {
	public:
		Sampler(const CreateInfo& createInfo);
		virtual ~Sampler() override;

		VkSampler GetSampler() const;

	protected:
		VkSampler sampler = nullptr;
	};
}
