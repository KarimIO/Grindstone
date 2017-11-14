#ifndef _SGEOMETRY_H
#define _SGEOMETRY_H

#include "../GraphicsCommon/GraphicsWrapper.h"
#include "../GraphicsCommon/Texture.h"
#include "../GraphicsCommon/CommandBuffer.h"
#include "../GraphicsCommon/VertexArrayObject.h"

#include <glm/glm.hpp>
#include <vector>

#include "SMaterial.h"

enum GeometryType {
	GEOMETRY_STATIC_MODEL = 0,
	GEOMETRY_SKELETAL_MODEL,
	GEOMETRY_TERRAIN
};

class Material;

class CRender {
private:
public:
	size_t entityID;
	std::vector<Material *> materials;
};

class Mesh {
public:
	MaterialReference material;

	virtual void Draw() = 0;
	virtual void DrawDeferred(CommandBuffer *) = 0;
};

class CGeometry {};

class SSubGeometry {
public:
	virtual CGeometry *PreloadComponent(std::string path) = 0;
	virtual CGeometry *LoadComponent(std::string path) = 0;
	virtual void LoadPreloaded() = 0;
	virtual ~SSubGeometry() = 0;
};

class SGeometry {
public:
	void AddComponent(GeometryType type, std::string path);
	void AddSystem(SSubGeometry *system);
	void LoadPreloaded();
	~SGeometry();
private:
	std::vector<SSubGeometry *> systems;
};

#endif