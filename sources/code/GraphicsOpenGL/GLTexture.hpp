#ifndef _GL_TEXTURE_H
#define _GL_TEXTURE_H

#include "../GraphicsCommon/Texture.hpp"
#include <vector>

void TranslateFormats(ImageFormat inFormat, GLenum &format, GLint &internalFormat);

class GLTexture : public Texture {
	GLuint handle;
	bool isCubemap;
public:
	GLTexture(TextureCreateInfo ci);
	GLTexture(CubemapCreateInfo ci);
	void Bind(int i);
	~GLTexture();
};

class GLTextureBinding : public TextureBinding {
	std::vector<GLTexture *> textures;
	std::vector<uint32_t> targets;
public:
	GLTextureBinding(TextureBindingCreateInfo ci);
	void Bind();
};

class GLTextureBindingLayout : public TextureBindingLayout {
public:
	TextureSubBinding *subbindings;
	uint32_t subbindingCount;
	GLTextureBindingLayout(TextureBindingLayoutCreateInfo);
	TextureSubBinding GetSubBinding(uint32_t i);
	uint32_t GetNumSubBindings();
};

#endif