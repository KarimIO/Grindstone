#ifndef _S_SHADER_H
#define _S_SHADER_H

#include "GraphicsWrapper.hpp"
#include "SGeometry.hpp"
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

enum ProgramType {
	TYPE_UNLIT = 0,
	TYPE_OPAQUE,
	TYPE_TRANSPARENT
};

bool readFile(const std::string& filename, std::vector<char>& outputfile);

class ParameterDescriptor {
	public:
	std::string description;
	PARAM_TYPE paramType;
	void *dataPtr;
	ParameterDescriptor();
	ParameterDescriptor(std::string _desc, PARAM_TYPE _type, void *_ptr);
};

class TextureParameterDescriptor : public ParameterDescriptor {
public:
	unsigned int texture_id;
	TextureParameterDescriptor() {};
	TextureParameterDescriptor(unsigned int _texture_id, std::string _desc, PARAM_TYPE _type, void *_ptr);
};

class Material;

struct PipelineReference {
	uint8_t renderpass = 0;
	uint8_t pipeline = 0;
	ProgramType pipeline_type;
};

struct MaterialReference {
	PipelineReference pipelineReference;
	uint16_t material = 0;
};

class Mesh;

class MaterialManager;

class Material {
	friend MaterialManager;
public:
	TextureBinding *m_textureBinding;
	std::vector<Mesh *> m_meshes;
	Material() {
		m_textureBinding = 0;
		m_meshes.clear();
	}
	Material(MaterialReference reference, TextureBinding *textureBinding);
	void IncrementDrawCount();
private:
	MaterialReference reference;
	uint32_t draw_count;
};

struct PipelineContainer {
	ProgramType type;
	TextureBindingLayout *tbl;
	PipelineReference reference;
	GraphicsPipeline *program;
	GraphicsPipeline *shadow_program;
	CommandBuffer *commandBuffer;
	std::string name;
	std::string name_text;
	std::string shader_paths[SHADER_FRAGMENT + 1];
	std::string shadow_shader_paths[SHADER_FRAGMENT + 1];
	std::map<std::string, TextureParameterDescriptor> textureDescriptorTable;
	std::map<std::string, ParameterDescriptor> parameterDescriptorTable;
	std::vector<Material> materials;
	uint32_t draw_count;
};

struct RenderPassContainer {
public:
	RenderPass *renderPass;
	CommandBuffer *commandBuffer;
	std::vector<Framebuffer *> framebuffers;
	std::vector<PipelineContainer> pipelines_deferred;
	std::vector<PipelineContainer> pipelines_unlit;
	std::vector<PipelineContainer> pipelines_forward;
};

struct GeometryInfo {
	VertexBindingDescription *vbds;
	unsigned int vbds_count;
	VertexAttributeDescription *vads;
	unsigned int vads_count;
	UniformBufferBinding **ubbs;
	unsigned int ubb_count;
};

class MaterialManager {
public:
	RenderPassContainer *Initialize(GraphicsWrapper *gw);
	PipelineReference CreatePipeline(GeometryInfo geometry_info, std::string pipelineName);
	MaterialReference CreateMaterial(GeometryInfo geometry_info, std::string shaderName);
	MaterialReference PreLoadMaterial(GeometryInfo geometry_info, std::string shaderName);

	Texture * LoadCubemap(std::string path);

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
	void DrawUnlitImmediate();
	void DrawShadowsImmediate();
	void DrawDeferredImmediate();
	void DrawForwardImmediate();
	void DrawDeferredCommand();
	void generateProgram(GeometryInfo geometry_info, PipelineContainer &container);
	void resetDraws();
	~MaterialManager();
private:
	std::vector<RenderPassContainer> render_passes_;
	std::map<std::string, PipelineReference> pipeline_map_;
	std::map<std::string, MaterialReference> material_map_;
	std::map<std::string, Texture *> texture_map_;
	std::vector<Texture *> unloaded_;

	GraphicsWrapper *graphics_wrapper_;
};

#endif