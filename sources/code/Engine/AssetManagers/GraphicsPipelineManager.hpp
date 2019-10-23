#ifndef _GRAPHICS_PIPELINE_MANAGER_H
#define _GRAPHICS_PIPELINE_MANAGER_H

#include <map>
#include <vector>
#include "AssetReferences.hpp"
#include "GraphicsPipeline.hpp"

class GraphicsWrapper;
class GraphicsPipeline;
class CommandBuffer;
class Texture;
class TextureBindingLayout;
class RenderPass;
class Framebuffer;
class UniformBufferBinding;

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
class MaterialManager;

struct PipelineContainer {
	ProgramType type;
	TextureBindingLayout *tbl;
	TextureBindingLayout **tbl_arr;
	PipelineReference reference;
	GraphicsPipeline *program;
	GraphicsPipeline *shadow_program;
	CommandBuffer *commandBuffer;

	CullMode cull_mode_;
	std::string name;
	std::string name_text;
	std::string shader_paths[SHADER_FRAGMENT + 1];
	std::string shadow_shader_paths[SHADER_FRAGMENT + 1];
	std::map<std::string, TextureParameterDescriptor> textureDescriptorTable;
	std::map<std::string, ParameterDescriptor> parameterDescriptorTable;
	std::vector<Material> materials;
	uint32_t draw_count;
	uint16_t param_size;
	UniformBufferBinding *param_ubb;
};

struct RenderPassContainer {
public:
	RenderPass *renderPass;
	CommandBuffer *commandBuffer;
	std::vector<Framebuffer *> framebuffers;
	std::vector<PipelineContainer> pipelines_deferred;
	std::vector<PipelineContainer> pipelines_unlit;
	std::vector<PipelineContainer> pipelines_forward;
	std::vector<PipelineContainer> pipelines_misc;
};

class GraphicsPipelineManager {
public:
	GraphicsPipelineManager();
	void initialize();
	PipelineReference createPipeline(GeometryInfo geometry_info, std::string pipelineName, bool miscPipeline = false);

	RenderPassContainer *getRenderPass(uint8_t);
	PipelineContainer *getPipeline(PipelineReference);

	void refresh();

	void removeRenderPass(uint8_t i);
	void removePipeline(PipelineReference);

	void loadPreloaded();
	//GraphicsPipeline *ParseShaderFile(std::string path);
	//GraphicsPipeline *CreateShaderFromPaths(std::string name, std::string vsPath, std::string fsPath, std::string gsPath, std::string csPath, std::string tesPath, std::string tcsPath);
	void drawUnlitImmediate();
	void drawShadowsImmediate(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	void drawDeferredImmediate();
	void drawForwardImmediate();
	// void drawDeferredCommand();
	void generateProgram(GeometryInfo geometry_info, PipelineContainer &container);
	void resetDraws();
	void cleanup();
	void destroyPipelineContainerGraphics(PipelineContainer &p);
	~GraphicsPipelineManager();
private:
	std::vector<RenderPassContainer> render_passes_;
	std::map<std::string, PipelineReference> pipeline_map_;
	std::map<std::string, MaterialReference> material_map_;
	std::map<std::string, Texture *> texture_map_;
	std::vector<Texture *> unloaded_;
};

#endif