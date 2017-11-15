#include "PostPipeline.hpp"

void PostPipeline::AddPostProcess(BasePostProcess *process) {
	processes.push_back(process);
}

Framebuffer *PostPipeline::ProcessScene(Framebuffer *inputFbo) {
	Framebuffer *fbo = inputFbo;
	for (auto &process : processes) {
		fbo = process->Process(fbo);
	}

	return fbo;
}

PostPipeline::~PostPipeline() {
	for (auto &process : processes) {
		delete process;
	}
}
