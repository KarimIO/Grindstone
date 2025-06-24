#pragma once

#include <stdint.h>
#include <vector>

#include <Common/Graphics/Sampler.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Sampler : public Grindstone::GraphicsAPI::Sampler {
	public:
		Sampler(const CreateInfo& createInfo);
		GLuint GetSampler() const;
		virtual ~Sampler();

	private:
		GLuint sampler;
	};
}
