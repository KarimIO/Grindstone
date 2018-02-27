#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "SMaterial.hpp"

class Skybox {
public:
    void Initialize(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper);
    void Render();
    GeometryInfo geometry_info_;
private:
    MaterialReference material;
    MaterialManager *material_system_;
    GraphicsWrapper *graphics_wrapper_;

	VertexArrayObject *planeVAO;
	VertexBuffer *planeVBO;
};

#endif // !SKYBOX_HPP