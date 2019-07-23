#ifndef _POST_PIPELINE_H
#define _POST_PIPELINE_H

#include "Framebuffer.hpp"
#include "BasePost.hpp"

class Space;

// Contains all active post-process pipelines for a camera.
// Note: Should ideally exist per-camera, as an attribute of it.
// Example:
// 		PostPipeline process;
// 		process.AddPostProcess(colorGrading);
// 		process.AddPostProcess(dofProcess);
// 		process.AddPostProcess(blurProcess);
// 		...
//		fbo = process.ProcessScene(fbo);
class PostPipeline {
public:
	PostPipeline(Space *space);
	void Reserve(unsigned int size);
	void AddPostProcess(BasePostProcess *);
	void Process();
	void setSpace(Space *space);
	Space *getSpace();
	~PostPipeline();
private:
	std::vector<BasePostProcess *> processes_;
	Space *space_;
};

#endif