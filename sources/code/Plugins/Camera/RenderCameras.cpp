#include "RenderCameras.hpp"
#include <EngineCore/Scenes/Scene.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <Common/Graphics/VertexBuffer.hpp>
#include <Common/Graphics/IndexBuffer.hpp>
#include <fstream>
using namespace Grindstone;

struct Vertex {
    float pos[3];
    float color[3];
    float uv[2];
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.5f}, {0.3f,  0.65f, 1.0f }, {0.0f, 0.0f}},
    {{-0.5f,  0.5f, 0.5f}, {0.3f,  0.65f, 1.0f }, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.5f}, {0.65f, 1.0f,  1.0f }, {1.0f, 1.0f}},
    {{ 0.5f, -0.5f, 0.5f}, {0.3f,  1.0f,  0.65f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f,-0.5f}, {0.65f, 1.0f,  0.3f }, {1.0f, 1.0f}},
    {{-0.5f,  0.5f,-0.5f}, {0.65f, 1.0f,  0.3f }, {0.0f, 1.0f}},
    {{ 0.5f,  0.5f,-0.5f}, {1.0f,  0.3f,  0.65f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,-0.5f}, {1.0f,  0.65f, 0.3f }, {1.0f, 0.0f}}
};

const uint16_t indices[] = {
    0, 1, 2,
    0, 2, 3,
    4, 5, 6,
    4, 6, 7
};

void loadShader(const char* path, GraphicsAPI::ShaderStage ss, GraphicsAPI::ShaderStageCreateInfo& ssci) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    ssci.fileName = path;
    ssci.content = new char[fileSize];
    ssci.size = static_cast<uint32_t>(fileSize);
    ssci.type = ss;

    file.seekg(0);
    file.read((char*)ssci.content, fileSize);

    file.close();
}

CameraRenderingSystem::CameraRenderingSystem(Scene* s) : transform_array_(*(ECS::ComponentArray<TransformComponent>*)s->getECS()->getComponentArray("Transform")), camera_array_(*(ECS::ComponentArray<CameraComponent>*)s->getECS()->getComponentArray("Camera")) {
    scene_ = s;
}

void CameraRenderingSystem::setGraphicsCore(GraphicsAPI::Core* core, Window* win) {
    graphics_core_ = core;
    window_ = win;

    GraphicsAPI::VertexBufferLayout vbd({
        { GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, GraphicsAPI::AttributeUsage::Position },
        //{ GraphicsAPI::VertexFormat::Float3, "vertexColor", false, GraphicsAPI::AttributeUsage::Color },
        //{ GraphicsAPI::VertexFormat::Float2, "vertexTexCoord", false, GraphicsAPI::AttributeUsage::TexCoord0 }
    });

    GraphicsAPI::VertexBuffer::CreateInfo vbci;
    vbci.layout = &vbd;
    vbci.content = static_cast<const void*>(vertices.data());
    vbci.count = static_cast<uint32_t>(vertices.size());
    vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());
    auto vertex_buffer = graphics_core_->createVertexBuffer(vbci);

    GraphicsAPI::IndexBuffer::CreateInfo ibci;
    ibci.count = 3 * 4;
    ibci.content = indices;
    ibci.size = sizeof(indices);
    auto index_buffer = graphics_core_->createIndexBuffer(ibci);

    GraphicsAPI::VertexArrayObject* vao = nullptr;
    if (graphics_core_->shouldUseImmediateMode()) {
        GraphicsAPI::VertexArrayObject::CreateInfo vao_ci{ &vertex_buffer, 1, index_buffer };
        vao = graphics_core_->createVertexArrayObject(vao_ci);
    }

    GraphicsAPI::ClearColorValue clear_val = { 40.0f, 60.0f, 80.0f, 255.0f };

    auto colorformat = GraphicsAPI::ColorFormat::R8G8B8A8; // gr->getDeviceColorFormat();
    GraphicsAPI::RenderPass::CreateInfo rpci = {};
    rpci.color_format_count_ = 1;
    rpci.color_formats_ = &colorformat;
    rpci.depth_format_ = GraphicsAPI::DepthFormat::D24;
    rpci.width_ = 800;
    rpci.height_ = 600;
    auto rp = graphics_core_->createRenderPass(rpci);

    GraphicsAPI::Pipeline::CreateInfo gpci;
    gpci.vertex_bindings = &vbd;
    gpci.vertex_bindings_count = 1;
    gpci.cullMode = GraphicsAPI::CullMode::None;
    gpci.width = 800;
    gpci.height = 600;
    gpci.renderPass = rp;
    gpci.primitiveType = GraphicsAPI::GeometryType::Triangles;
    gpci.scissorX = 0;
    gpci.scissorY = 0;
    gpci.scissorW = 800;
    gpci.scissorH = 600;
    gpci.uniformBufferBindingCount = 0;
    gpci.uniformBufferBindings = nullptr;
    gpci.textureBindingCount = 0;
    gpci.textureBindings = nullptr;
    gpci.shaderStageCreateInfoCount = 2;
    gpci.shaderStageCreateInfos = new GraphicsAPI::ShaderStageCreateInfo[2];
    std::string vert = "../vert.spv";
    std::string frag = "../frag.spv";

    loadShader(vert.c_str(), GraphicsAPI::ShaderStage::Vertex, gpci.shaderStageCreateInfos[0]);
    loadShader(frag.c_str(), GraphicsAPI::ShaderStage::Fragment, gpci.shaderStageCreateInfos[1]);
    auto gp = graphics_core_->createPipeline(gpci);
    delete gpci.shaderStageCreateInfos[0].content;
    delete gpci.shaderStageCreateInfos[1].content;

    float clear_color[4] = { 0.1f, 0.2f, 0.3f, 1.0f };

    while (true) {
        win->immediateSetContext();
        graphics_core_->clear(GraphicsAPI::ClearMode::All, clear_color, 1.0f, 0);
        gp->bind();
        graphics_core_->bindVertexArrayObject(vao);
        graphics_core_->drawImmediateIndexed(GraphicsAPI::GeometryType::Triangles, false, 0, 0, 3 * 4);
        win->immediateSwapBuffers();
    }
}

void CameraRenderingSystem::update() {
    // For each camera
    {
        CameraComponent& camera = camera_array_.getComponent(1);
        TransformComponent& transform = transform_array_.getComponent(1);

        glm::mat4 perspective, view;
        perspective = glm::perspective(
            camera.fov_, // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
            4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
            camera.near_,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
            camera.far_             // Far clipping plane. Keep as little as possible.
        );

        view = glm::lookAt(
            glm::vec3(1,1,1), // the position of your camera, in world space
            glm::vec3(0,0,0),   // where you want to look at, in world space
            glm::vec3(0,1,0)       // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
        );

        // Update uniform buffer

        // Cull geometry

        // Render all renderpasses
    }
}
