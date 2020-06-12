#pragma once

#include <GraphicsCommon/VertexArrayObject.hpp>
#include <GraphicsCommon/VertexBuffer.hpp>
#include <GraphicsCommon/IndexBuffer.hpp>
#include <glm/glm.hpp>
#include <AssetCommon/Renderable.hpp>

struct Vertex2D {
    glm::vec2 position;
    glm::vec4 color;
    glm::vec2 tex_coord;
};

struct Quad2D {
    Vertex2D vertices[4];
};

class Renderer2D : public Renderable {
public:
    void setMaterial(std::string path);
    virtual void shadowDraw() override;
    virtual void draw() override;
public:
    void resize(unsigned int size);
    void updateBuffers();
    unsigned int requestQuadSlot();
    Quad2D &getQuadMemory(unsigned int);
    void freeQuad(unsigned int size);
    ~Renderer2D();
private:
    bool dirty_;
    unsigned int count_;
    unsigned int capacity_;
    Quad2D* vertex_buffer_;
    unsigned int* indices_buffer_;
    bool* free_quad_list_;
    MaterialReference material_reference_;
    Grindstone::GraphicsAPI::VertexArrayObject *vao_ = nullptr;
    Grindstone::GraphicsAPI::VertexBuffer *vbo_ = nullptr;
    Grindstone::GraphicsAPI::IndexBuffer *ibo_ = nullptr;
    GeometryInfo geometry_info_;
    Grindstone::GraphicsAPI::VertexBufferLayout vertex_layout_;
};

class Renderer2DManager {
public:
    void initialize();
    ~Renderer2DManager();
private:
    GeometryInfo geometry_info_;
    Grindstone::GraphicsAPI::VertexBufferLayout vertex_layout_;
};