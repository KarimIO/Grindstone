#pragma once

#include <Common/Graphics/ComputePipeline.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class ComputePipeline : public Grindstone::GraphicsAPI::ComputePipeline {
	public:
		ComputePipeline(const CreateInfo& createInfo);
		virtual void Recreate(const CreateInfo& createInfo) override;
		void Bind();
		~ComputePipeline();
	private:
		void CreatePipeline(const CreateInfo& createInfo);

		GLuint program;
	};
}
