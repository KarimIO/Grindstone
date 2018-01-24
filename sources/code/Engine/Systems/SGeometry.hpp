#ifndef _SGEOMETRY_H
#define _SGEOMETRY_H

#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "../GraphicsCommon/Texture.hpp"
#include "../GraphicsCommon/CommandBuffer.hpp"
#include "../GraphicsCommon/VertexArrayObject.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "CBase.hpp"

#include "SMaterial.hpp"
#include "SCamera.hpp"

enum GeometryType {
	GEOMETRY_STATIC_MODEL = 0,
	//GEOMETRY_SKELETAL_MODEL,
	GEOMETRY_TERRAIN
};

class Material;
struct MaterialReference;

class Mesh {
public:
	Material *material;

	virtual void ShadowDraw() = 0;
	virtual void Draw() = 0;
	virtual void DrawDeferred(CommandBuffer *) = 0;
};

class CRender {
public:
	uint32_t entity_id;
	GeometryType geometry_type;
	uint32_t geometry_id;
	std::vector<Material *> materials;
	bool should_draw;
};

class Geometry {
public:
	virtual void setName(std::string dir) = 0;
};

class SSubGeometry {
public:
	virtual void LoadGeometry(unsigned int render_id, std::string path) = 0;
	virtual void LoadPreloaded() = 0;
	virtual void Cull(CCamera *cam) = 0;
	virtual ~SSubGeometry() {};
};

class SGeometry {
public:
	void AddComponent(uint32_t entityID, uint32_t &component, GeometryType type);
	CRender &GetComponent(uint32_t id);
	void RemoveComponent(uint32_t id);
	void AddSystem(SSubGeometry *system);
	void LoadPreloaded();
	void Cull(CCamera *cam);
	SSubGeometry *GetSystem(uint32_t id);
	~SGeometry();
private:
	std::vector<SSubGeometry *> systems_;
	std::vector<CRender> render_components_;
};

#endif