#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class UniformBufferBinding {
		public:
			struct CreateInfo {
				const char* shaderLocation;
				uint32_t binding;
				ShaderStageBit stages;
				uint32_t size;
			};
		};

		class UniformBuffer {
		public:
			struct CreateInfo {
				bool isDynamic;
				uint32_t size;
				UniformBufferBinding* binding;
			};

			virtual void UpdateBuffer(void * content) = 0;
			virtual void Bind() = 0;
		};
	};
};
