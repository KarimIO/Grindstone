#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLDescriptorSetLayout : public DescriptorSetLayout {
		public:
			GLDescriptorSetLayout(CreateInfo& createInfo);
			virtual ~GLDescriptorSetLayout();
		};
	}
}
