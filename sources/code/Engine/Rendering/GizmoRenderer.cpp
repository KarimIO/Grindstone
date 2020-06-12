#include <Rendering/GizmoRenderer.hpp>
#include <Core/Engine.hpp>
#include <Core/Utilities.hpp>

namespace Grindstone {
	GizmoRenderer::DrawPointVertex::DrawPointVertex() : pos(0.f, 0.f, 0.f), color(0.f, 0.f, 0.f, 1.f) {}
	GizmoRenderer::DrawPointVertex::DrawPointVertex(DrawPointVertex &other) : pos(other.pos), color(other.color) {}
	GizmoRenderer::DrawPointVertex::DrawPointVertex(glm::vec3 _pos, glm::vec4 _color) : pos(_pos), color(_color) {}

    GizmoRenderer::DrawableDebugLine::DrawableDebugLine() : time_end(0.f) {}
    GizmoRenderer::DrawableDebugLine::DrawableDebugLine(DrawableDebugLine& other) : from(other.from), to(other.to), time_end(other.time_end) {}
    GizmoRenderer::DrawableDebugLine::DrawableDebugLine(DrawPointVertex _from, DrawPointVertex _to, float _time_end) : from(_from), to(_to), time_end(_time_end) {}
    GizmoRenderer::DrawableDebugLine::DrawableDebugLine(glm::vec3 from_pos, glm::vec4 from_color, glm::vec3 to_pos, glm::vec4 to_color, float _time_end) : from(from_pos, from_color), to(to_pos, to_color), time_end(_time_end) {}

	void GizmoRenderer::initialize() {
		auto gw = engine.getGraphicsWrapper();

		// Prepare Vertex Layout
		vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
			{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
			{ Grindstone::GraphicsAPI::VertexFormat::Float4, "vertexColor", false, Grindstone::GraphicsAPI::AttributeUsage::Color }
		});

		// Prepare GraphicsPipeline
		Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
		Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
		if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
			vi.fileName = "../assets/shaders/debug/line.vert.glsl";
			fi.fileName = "../assets/shaders/debug/line.frag.glsl";
		}
		else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
			vi.fileName = "../assets/shaders/debug/line.vert.fxc";
			fi.fileName = "../assets/shaders/debug/line.frag.fxc";
		}
		else {
			vi.fileName = "../assets/shaders/debug/line.vert.spv";
			fi.fileName = "../assets/shaders/debug/line.frag.spv";
		}

		std::vector<char> vfile;
		if (!readFile(vi.fileName, vfile))
			return;
		vi.content = vfile.data();
		vi.size = (uint32_t)vfile.size();
		vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

		std::vector<char> ffile;
		if (!readFile(fi.fileName, ffile))
			return;
		fi.content = ffile.data();
		fi.size = (uint32_t)ffile.size();
		fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

		std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

		Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo gpci;
		gpci.cullMode = Grindstone::GraphicsAPI::CullMode::None;
		gpci.vertex_bindings = &vertex_layout_;
		gpci.vertex_bindings_count = 2;
		gpci.width = (float)engine.getSettings()->resolution_x_;
		gpci.height = (float)engine.getSettings()->resolution_y_;
		gpci.scissorW = engine.getSettings()->resolution_x_;
		gpci.scissorH = engine.getSettings()->resolution_y_;
		gpci.primitiveType = Grindstone::GraphicsAPI::GeometryType::Lines;
		gpci.shaderStageCreateInfos = stages.data();
		gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
		gpci.textureBindings = nullptr;
		gpci.textureBindingCount = 0;
		std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs = { engine.getUniformBufferBinding() };
		gpci.uniformBufferBindings = ubbs.data();
		gpci.uniformBufferBindingCount = (uint32_t)ubbs.size();
		line_gp_ = gw->createGraphicsPipeline(gpci);
	}

	void GizmoRenderer::render() {
		// Set Blending
		engine.getGraphicsWrapper()->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::AdditiveAlpha);

		// Set Uniform Buffer
		engine.getUniformBuffer()->bind();

		// Draw without Depth
		engine.getGraphicsWrapper()->enableDepth(false);
		renderLines(false);

		// Draw with Depth
		engine.getGraphicsWrapper()->enableDepth(true);
		renderLines(true);
	}

	void GizmoRenderer::renderLines(bool use_depth) {
		std::vector<DrawableDebugLine>* draw_line;
		if (use_depth) {
			draw_line = &debug_lines_depth_;
		}
		else {
			draw_line = &debug_lines_;
		}

		if (draw_line->size() == 0)
			return;

		auto gw = engine.getGraphicsWrapper();
		auto time = engine.getTimeCurrent();

		// Prepare Shader
		line_gp_->Bind();

		// Prepare Vertices
		std::vector<DrawPointVertex> line_vertices;
		for (int i = draw_line->size() - 1; i >= 0; --i) {
			auto &debug_line = draw_line->at(i);

			if (time > debug_line.time_end) {
				draw_line->erase(draw_line->begin() + i);
			}
			else {
				line_vertices.emplace_back(debug_line.from);
				line_vertices.emplace_back(debug_line.to);
			}
		}

		if (line_vertices.size() == 0) return;

		// Prepare Buffer
		Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
		vbci.layout = &vertex_layout_;
		vbci.content = line_vertices.data();
		vbci.count = line_vertices.size();
		vbci.size = sizeof(DrawPointVertex) * vbci.count;
		auto vbo = gw->createVertexBuffer(vbci);
		vbo->updateBuffer(line_vertices.data());

		Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaoci;
		vaoci.vertex_buffers = &vbo;
		vaoci.vertex_buffer_count = 1;
		vaoci.index_buffer = nullptr;
		auto vao = gw->createVertexArrayObject(vaoci);

		// Draw
		gw->bindVertexArrayObject(vao);
		gw->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, line_vertices.size());

		// Cleanup
		gw->deleteVertexBuffer(vbo);
		gw->deleteVertexArrayObject(vao);
	}

	void GizmoRenderer::drawLine(glm::vec3 from, glm::vec3 to, glm::vec4 from_color, glm::vec4 to_color, float duration, bool use_depth) {
		float time = engine.getTimeCurrent();

		if (use_depth) {
			std::cout << "Depth debug for lines doesn't work yet.\r\n";
			debug_lines_depth_.emplace_back(from, from_color, to, to_color, time + duration);
		}
		else {
			debug_lines_.emplace_back(from, from_color, to, to_color, time + duration);
		}
	}

	void GizmoRenderer::drawRay(glm::vec3 from, glm::vec3 dir, glm::vec4 from_color, glm::vec4 to_color, float duration, bool use_depth) {
		drawLine(from, from + dir * 20000.f, from_color, to_color, duration, use_depth);
	}
};