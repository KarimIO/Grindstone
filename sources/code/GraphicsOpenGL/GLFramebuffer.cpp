#include "gl3w.h"
#include <iostream>
#include "GLFramebuffer.h"

GRAPHICS_EXPORT Framebuffer* createFramebuffer() {
	return new GLFramebuffer;
}

void GLFramebuffer::Initialize(unsigned short numBuffer) {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	numBuffers = numBuffer;
	targetBuffer = 0;
	textures = new unsigned int[numBuffers];
	glGenTextures(numBuffers, textures);
}

// Eventually have the following two only use one type parameter, or organize it better.
void GLFramebuffer::AddBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height) {
	glBindTexture(GL_TEXTURE_2D, textures[targetBuffer]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, colorType, width, height, 0, colorFormat, colorDataType, NULL);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targetBuffer, GL_TEXTURE_2D, textures[targetBuffer], 0);
	targetBuffer++;
}

void GLFramebuffer::AddCubeBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[targetBuffer]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, colorType, width, height, 0, colorFormat, colorDataType, NULL);
	}

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targetBuffer, GL_TEXTURE_2D, textures[targetBuffer], 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	targetBuffer++;
}

void GLFramebuffer::AddDepthBuffer(unsigned int width, unsigned int height) {
	glGenTextures(1, &renderBuffer);
	glBindTexture(GL_TEXTURE_2D, renderBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderBuffer, 0);
}

void GLFramebuffer::Generate() {
	GLenum *DrawBuffers = new GLenum[numBuffers];
	for (size_t i = 0; i < numBuffers; i++)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

	glDrawBuffers(numBuffers, DrawBuffers);

	// Report errors
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Framebuffer Error. Status 0x%x\n", status);
	}

	// Unbind
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLFramebuffer::BindTexture(unsigned int fboLoc) {
	glActiveTexture(GL_TEXTURE0 + fboLoc);
	glBindTexture(GL_TEXTURE_2D, textures[fboLoc]);
}

void GLFramebuffer::WriteBind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void GLFramebuffer::ReadBind() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
}

void GLFramebuffer::Unbind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
