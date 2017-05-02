#include "PostPipeline.h"

void PostPipeline::Initialize() {
	colorGrading.Initialize();
}

void PostPipeline::Start() {
}

void PostPipeline::End() {
}

void PostPipeline::ProcessScene(Framebuffer *fbo) {
	Start();

	colorGrading.Process(fbo);

	End();
}
