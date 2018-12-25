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
	for (int i = 0; i < processes_.size(); i++) {
		processes_.at(i)->Process();
	}
}

Space *PostPipeline::getSpace() {
	return space_;
}

PostPipeline::~PostPipeline() {
}
