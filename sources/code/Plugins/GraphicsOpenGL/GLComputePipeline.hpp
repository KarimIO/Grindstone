#pragma once

#include <Common/Graphics/ComputePipeline.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLComputePipeline : public ComputePipeline {
		public:
			GLComputePipeline(ComputePipeline::CreateInfo& createInfo);
			virtual void Recreate(ComputePipeline::CreateInfo& createInfo) override;
			void Bind();
			~GLComputePipeline();
		private:
			void CreatePipeline(ComputePipeline::CreateInfo& createInfo);

			GLuint program;
		};
	}
}
