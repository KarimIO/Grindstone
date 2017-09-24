#ifndef _POST_PIPELINE_H
#define _POST_PIPELINE_H

#include "Framebuffer.h"
#include "ColorGradingPost.h"

class PostPipeline {
	ColorGradingPost colorGrading;
public:
	void Initialize();
	
	void Start();
	void End();

	void ProcessScene(Framebuffer *inputFbo, Framebuffer *fbo);
	//void ProcessSceneAndSecondary();
	//void ProcessSceneAndSecondaryAndGUI();
};

#endif