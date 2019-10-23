#include "BasePost.hpp"

BasePostProcess::BasePostProcess(PostPipeline * post) : pipeline_(post) {}

BasePostProcess::~BasePostProcess()
{
}

void BasePostProcess::resizeBuffers(unsigned int w, unsigned h) {

}

void BasePostProcess::reloadGraphics(unsigned int w, unsigned h) {

}

void BasePostProcess::destroyGraphics() {

}

PostPipeline * BasePostProcess::getPipeline() {
	return pipeline_;
}
