#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct UniformBufferBindingCreateInfo {
			const char *shaderLocation;
			uint32_t binding;
			ShaderStageBit stages;
			uint32_t size;
		};

		class UniformBufferBinding {
		public:
		};

		struct UniformBufferCreateInfo {
			bool isDynamic;
			uint32_t size;
			UniformBufferBinding *binding;
		};

		class UniformBuffer {
		public:
			virtual void UpdateUniformBuffer(void * content) {};
			virtual void Bind() {};
		};
	};
};
