#if 0
//#ifndef _S_GEOMETRY_STATIC_H
#define _S_GEOMETRY_STATIC_H

#include "SGeometry.hpp"
#include "../FormatCommon/Bounding.hpp"



class SGeometryStatic : public SSubGeometry {
public:
	SGeometryStatic(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper, std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs);

	virtual void LoadGeometry(uint32_t render_id, std::string path);
	virtual void LoadPreloaded();

	virtual void Cull(CCamera *cam);

	~SGeometryStatic();
private:
	GraphicsWrapper *graphics_wrapper_;
	MaterialManager *material_system_;
	GeometryInfo geometry_info_;
	std::vector<CommandBuffer *> GetCommandBuffers();

	std::vector<CModelStatic> models;
	std::vector<unsigned int> unloadedModelIDs;
	void LoadModel(CModelStatic *model);
};

#endif