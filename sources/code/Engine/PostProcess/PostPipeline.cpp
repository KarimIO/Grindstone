#include "PostPipeline.hpp"
#include "PostProcessTonemap.hpp"

PostPipeline::PostPipeline(Space * space) : space_(space) {}

void PostPipeline::Reserve(unsigned int size) {
	processes_.reserve(size);
}

void PostPipeline::AddPostProcess(BasePostProcess *process) {
	processes_.push_back(process);
}

void PostPipeline::Process() {
	GRIND_PROFILE_FUNC();
	for (int i = 0; i < processes_.size(); i++) {
		processes_.at(i)->Process();
	}
}

void PostPipeline::setSpace(Space * space) {
	space_ = space;
}

void PostPipeline::resizeBuffers(unsigned int w, unsigned int h) {
	for (auto p : processes_) {
		p->resizeBuffers(w, h);
	}
}

void PostPipeline::reloadGraphics(unsigned int w, unsigned int h) {
	for (auto p : processes_) {
		p->reloadGraphics(w, h);
	}
}

void PostPipeline::destroyGraphics() {
	for (auto p : processes_) {
		p->destroyGraphics();
	}
}

Space *PostPipeline::getSpace() {
	return space_;
}

PostPipeline::~PostPipeline() {
	for (auto p : processes_) {
		delete p;
	}
}
