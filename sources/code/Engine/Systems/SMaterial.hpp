#ifndef _S_SHADER_H
#define _S_SHADER_H

#include "GraphicsWrapper.h"
#include <map>
#include <vector>

enum PARAM_TYPE {
	PARAM_BOOL,
	PARAM_INT,
	PARAM_UINT,
	PARAM_FLOAT,
	PARAM_DOUBLE,
	PARAM_BVEC2,
	PARAM_BVEC3,
	PARAM_BVEC4,
	PARAM_IVEC2,
	PARAM_IVEC3,
	PARAM_IVEC4,
	PARAM_UVEC2,
	PARAM_UVEC3,
	PARAM_UVEC4,
	PARAM_VEC2,
	PARAM_VEC3,
	PARAM_VEC4,
	PARAM_DVEC2,
	PARAM_DVEC3,
	PARAM_DVEC4,
	PARAM_TEXTURE,
	PARAM_CUBEMAP
};

bool readFile(const std::string& filename, std::vector<char>& outputfile);

struct ParameterDescriptor {
	std::string description;
	PARAM_TYPE paramType;
	void *dataPtr;
	ParameterDescriptor(std::string _desc, PARAM_TYPE _type, void *_ptr);
	ParameterDescriptor();
};

class Material;

struct PipelineReference {
	uint8_t renderpass = 0;
	uint8_t pipeline = 0;
};

struct MaterialReference {
	PipelineReference pipelineReference;
	uint16_t material = 0;
};

class CModel;

class Material {
public:
	TextureBinding *m_textureBinding;
	std::vector<Mesh *> m_meshes;
	Material() {
		m_textureBinding = 0;
		m_meshes.clear();
	}
	Material(TextureBinding *textureBinding);
};

struct PipelineContainer {
	GraphicsPipeline *program;
	CommandBuffer *commandBuffer;
	std::map<std::string, ParameterDescriptor> parameterDescriptorTable;
	std::vector<Material> materials;
};

struct RenderPassContainer {
public:
	RenderPass *renderPass;
	CommandBuffer *commandBuffer;
	std::vector<Framebuffer *> framebuffers;
	std::vector<PipelineContainer> pipelines;
};

class MaterialManager {
public:
	RenderPassContainer *Initialize(GraphicsWrapper *gw, VertexBindingDescription vbd, std::vector<VertexAttributeDescription> vads, UniformBufferBinding *ubb);
	PipelineReference CreatePipeline(std::string pipelineName);
	MaterialReference CreateMaterial(std::string shaderName);
	MaterialReference PreLoadMaterial(std::string shaderName);

	RenderPassContainer *GetRenderPass(uint8_t);
	PipelineContainer *GetPipeline(PipelineReference);
	Material *GetMaterial(MaterialReference);

	void RemoveRenderPass(uint8_t i);
	void RemovePipeline(PipelineReference);
	void RemoveMaterial(MaterialReference);
	void RemoveMeshFromMaterial(MaterialReference, Mesh *);

	Texture *PreLoadTexture(std::string path);
	Texture *LoadTexture(std::string path);
	void LoadPreloaded();
	//GraphicsPipeline *ParseShaderFile(std::string path);
	//GraphicsPipeline *CreateShaderFromPaths(std::string name, std::string vsPath, std::string fsPath, std::string gsPath, std::string csPath, std::string tesPath, std::string tcsPath);
	void DrawImmediate();
	void DrawDeferred();
	~MaterialManager();
private:
	std::vector<RenderPassContainer> render_passes_;
	std::map<std::string, PipelineReference> pipeline_map_;
	std::map<std::string, MaterialReference> material_map_;
	std::map<std::string, Texture *> texture_map_;

	GraphicsWrapper *graphics_wrapper_;
};

#endif