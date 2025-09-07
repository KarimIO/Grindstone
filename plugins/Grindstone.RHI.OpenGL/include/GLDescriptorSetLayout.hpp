#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class DescriptorSetLayout : public Grindstone::GraphicsAPI::DescriptorSetLayout {
	public:
		DescriptorSetLayout(const CreateInfo& createInfo);
		virtual ~DescriptorSetLayout();
	};
}
