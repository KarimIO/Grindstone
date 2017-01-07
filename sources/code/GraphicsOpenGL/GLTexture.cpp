#include "gl3w.h"
#include "GLTexture.h"
#include <iostream>

void GLTexture::CreateTexture(unsigned char *pixels, PixelScheme scheme, uint32_t width, uint32_t height) {
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	if (scheme == COLOR_SRGB)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void GLTexture::CreateCubemap(unsigned char * pixels[], PixelScheme scheme, uint32_t width, uint32_t height) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	for (size_t i = 0; i < 6; i++) {
		if (scheme == COLOR_SRGB)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels[i]);
		else
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels[i]);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

int GLTexture::GetTextureLocation() {
	return textureID;
}

void GLTexture::Bind(int bindTo) {
	glActiveTexture(GL_TEXTURE0 + bindTo);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void GLTexture::BindCubemap(int bindTo) {
	glActiveTexture(GL_TEXTURE0 + bindTo);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void GLTexture::Cleanup() {
	glDeleteTextures(1, &textureID);
}

GRAPHICS_EXPORT Texture* createTexture() {
	return new GLTexture;
}