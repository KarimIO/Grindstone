#include "BasePost.hpp"

BasePostProcess::BasePostProcess(PostPipeline * post) : pipeline_(post) {}

PostPipeline * BasePostProcess::getPipeline() {
	return pipeline_;
}
