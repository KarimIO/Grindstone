#include "gl3w.h"
#include <iostream>
#include "GLFramebuffer.h"

GRAPHICS_EXPORT Framebuffer* createFramebuffer() {
	return new GLFramebuffer;
}

void GLFramebuffer::Initialize(unsigned short numBuffer) {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	numBuffers = numBuffer;
	if (numBuffer > 0) {
		targetBuffer = 0;
		textures = new unsigned int[numBuffers];
		glGenTextures(numBuffers, textures);
		//glEnable(GL_FRAMEBUFFER_SRGB);
	}
	else {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
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
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, colorType, width, height, 0, colorFormat, colorDataType, NULL);
	}

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targetBuffer, GL_TEXTURE_2D, textures[targetBuffer], 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	targetBuffer++;
}

void GLFramebuffer::AddDepthBuffer(unsigned int width, unsigned int height) {
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
}

void GLFramebuffer::AddDepthCubeBuffer(unsigned int width, unsigned int height) {
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);

	for (size_t i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLFramebuffer::Generate() {
	if (numBuffers > 0) {
		GLenum *DrawBuffers = new GLenum[numBuffers];
		for (size_t i = 0; i < numBuffers; i++)
			DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + (GLenum)i;

		glDrawBuffers(numBuffers, DrawBuffers);
	}

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

void GLFramebuffer::BindTexture(unsigned int fboLoc, unsigned int bindLoc) {
	glActiveTexture(GL_TEXTURE0 + bindLoc);
	glBindTexture(GL_TEXTURE_2D, textures[fboLoc]);
}


void GLFramebuffer::BindDepth(unsigned int loc) {
	glActiveTexture(GL_TEXTURE0 + loc);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
}


void GLFramebuffer::BindDepthCube(unsigned int loc) {
	glActiveTexture(GL_TEXTURE0 + loc);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
}

void GLFramebuffer::WriteBind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void GLFramebuffer::WriteBindFace(unsigned int attachment, unsigned int face) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthBuffer, 0);
	
	// All is fine.
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Shadow FBO is broken with code " << status << std::endl;
	}
}

void GLFramebuffer::ReadBind() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
}

void GLFramebuffer::Unbind() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glDisable(GL_FRAMEBUFFER_SRGB);
}

void GLFramebuffer::TestBlit(unsigned int x, unsigned int y, unsigned int srcWidth, unsigned int srcHeight, unsigned int width, unsigned int height, bool depth) {
	glBlitFramebuffer(0, 0, srcWidth, srcHeight, x, y, x + width, y + height, depth ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void GLFramebuffer::SetAttachment(unsigned int a) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + a);
}