#include <iostream>
#include <cmath>
#include <GL/gl3w.h>
#include <GL/glext.h>
#include <glm/glm.hpp>

#include "GLSampler.hpp"
#include "GLFormats.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::Sampler::Sampler(const Sampler::CreateInfo& createInfo) {
	const GraphicsAPI::SamplerOptions& options = createInfo.options;

	glCreateSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER,
		TranslateMinFilterToOpenGL(
			true, // TODO: Use existing mips here
			options.minFilter,
			options.mipFilter
		)
	);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, TranslateMagFilterToOpenGL(options.magFilter));

	// Set wrapping modes
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, TranslateWrapToOpenGL(options.wrapModeU));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, TranslateWrapToOpenGL(options.wrapModeV));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, TranslateWrapToOpenGL(options.wrapModeW));

	if (GL_EXT_texture_filter_anisotropic && options.anistropy != 0.0f) {
		GLfloat maxAniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		float anistropy = options.anistropy > maxAniso
			? maxAniso
			: options.anistropy;
		glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, anistropy);
	}

	// Optional: set LOD clamp and bias
	glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, options.mipBias);
	glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, options.mipMin);
	glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, options.mipMax);
}

uint32_t OpenGL::Sampler::GetSampler() const {
	return sampler;
}

OpenGL::Sampler::~Sampler() {
	if (sampler != 0) {
		glDeleteSamplers(1, &sampler);
	}
}
