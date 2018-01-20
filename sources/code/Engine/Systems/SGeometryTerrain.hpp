#ifndef _TERRAIN_H
#define _TERRAIN_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "SGeometry.hpp"

class CTerrain : public Mesh {
	friend class SGeometryStatic;
public:
	std::vector<uint32_t> references;
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
	VertexArrayObject *vertexArrayObject;
	CommandBuffer *commandBuffer;
	std::string name;
	unsigned int num_indices;

	Material *material;

	CTerrain(std::string path, std::vector<uint32_t> refs) : name(path), references(refs) {};

	virtual std::string getName();
	virtual void setName(std::string dir);

	virtual void Draw();
	virtual void DrawDeferred(CommandBuffer *);
};

class SGeometryTerrain : public SSubGeometry {
public:
	SGeometryTerrain(MaterialManager * material_system, GraphicsWrapper * graphics_wrapper, std::vector<UniformBufferBinding*> ubbs);
	void LoadModel(CTerrain *model);
	virtual void LoadGeometry(unsigned int render_id, std::string path);
	virtual void LoadPreloaded();
	virtual void Cull(CCamera *cam);
private:
	MaterialManager *material_system_;
	GraphicsWrapper *graphics_wrapper_;
	GeometryInfo geometry_info_;

	std::vector<CTerrain> models;
	std::vector<unsigned int> unloadedModelIDs;
};

#endif