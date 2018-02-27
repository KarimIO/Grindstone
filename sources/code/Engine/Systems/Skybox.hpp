#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "SMaterial.hpp"

class Skybox {
public:
    void Initialize(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper, VertexArrayObject *plane_vao_, VertexBuffer *plane_vbo_);
    void SetMaterial(std::string path);
    void Render();
    GeometryInfo geometry_info_;
private:
    MaterialReference material;
    MaterialManager *material_system_;
    GraphicsWrapper *graphics_wrapper_;

    VertexArrayObject *plane_vao_;
	VertexBuffer *plane_vbo_;

    bool enabled_;
};

#endif // !SKYBOX_HPP