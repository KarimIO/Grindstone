#include "RenderPathDeferred.h"
#include "GraphicsDLLPointer.h"
#include "Utilities.h"
#include "gl3w.h"

void RenderPathDeferred::GeometryPass() {
	graphicsWrapper->Clear(CLEAR_ALL);
	fbo->WriteBind();
	geometryCache->Draw();
}

void RenderPathDeferred::DeferredPass() {
	//fbo->ReadBind();
	fbo->Unbind();

	fbo->BindTexture(0);
	fbo->BindTexture(1);
	fbo->BindTexture(2);

	graphicsWrapper->Clear(CLEAR_COLOR);

	shader->Use();

	int val[3] = { 0,1,2 };

	shader->PassData(&val);
	shader->SetInteger();
	shader->SetInteger();
	shader->SetInteger();

	vaoQuad->Bind();
	graphicsWrapper->DrawVertexArray(4);
	vaoQuad->Unbind();
}

void RenderPathDeferred::PostPass() {
}

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * gw, SModel * gc) {
	float tempVerts[8] = {
		-1,-1,
		1,-1,
		-1, 1,
		1, 1,
	};

	graphicsWrapper = gw;
	geometryCache = gc;

	vaoQuad = pfnCreateVAO();
	vaoQuad->Initialize();
	vaoQuad->Bind();

	vboQuad = pfnCreateVBO();
	vboQuad->Initialize(1);
	vboQuad->AddVBO(tempVerts, sizeof(tempVerts), 2, SIZE_FLOAT, DRAW_STATIC);
	vboQuad->Bind(0);
	vaoQuad->Unbind();

	fbo = pfnCreateFramebuffer();
	fbo->Initialize(3);
	fbo->AddBuffer(GL_RGB32F, GL_RGB, GL_FLOAT, 1024, 768);
	fbo->AddBuffer(GL_RGB32F, GL_RGB, GL_FLOAT, 1024, 768);
	fbo->AddBuffer(GL_RGB32F, GL_RGB, GL_FLOAT, 1024, 768);
	// Depth Buffer Issue:
	//fbo->AddDepthBuffer(1024, 768);
	fbo->Generate();

	std::string vsPath = "shaders/deferred.glvs";
	std::string fsPath = "shaders/deferred.glfs";

	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed To Read File.\n");

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed To Read File.\n");

	shader = pfnCreateShader();
	shader->AddShader(vsPath, vsContent, SHADER_VERTEX);
	shader->AddShader(fsPath, fsContent, SHADER_FRAGMENT);
	shader->Compile();

	shader->SetNumUniforms(3);
	shader->CreateUniform("texPos");
	shader->CreateUniform("texNormal");
	shader->CreateUniform("texAlbedo");
}

void RenderPathDeferred::Draw() {
	GeometryPass();
	DeferredPass();
	PostPass();
}
