#pragma once

#include <vector>
#include <glm/glm.hpp>

// Grindstone Includes
#include <GraphicsCommon/VertexArrayObject.hpp>
#include <GraphicsCommon/GraphicsPipeline.hpp>

namespace Grindstone {
	class GizmoRenderer {
	public:
		void initialize();
		void render();

	public:
		// Draw Commands
		void drawLine(glm::vec3 from, glm::vec3 to = glm::vec3(0.f, 0.f, 0.f), glm::vec4 from_color = glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4 to_color = glm::vec4(1.f, 0.f, 0.f, 1.f), float duration = 1.f, bool use_depth = false);
		void drawRay(glm::vec3 from, glm::vec3 dir = glm::vec3(1.f, 0.f, 0.f), glm::vec4 from_color = glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4 to_color = glm::vec4(1.f, 0.f, 0.f, 1.f), float duration = 1.f, bool use_depth = false);
	private:
		void renderLines(bool use_depth);
		struct DrawPointVertex {
			DrawPointVertex();
			DrawPointVertex(DrawPointVertex &other);
			DrawPointVertex(glm::vec3 pos, glm::vec4 color);
			glm::vec3 pos;
			glm::vec4 color;
		};

		struct DrawableDebugLine {
			DrawableDebugLine();
			DrawableDebugLine(DrawableDebugLine& other);
			DrawableDebugLine(DrawPointVertex from, DrawPointVertex to, float time_end);
			DrawableDebugLine(glm::vec3 from_pos, glm::vec4 from_color, glm::vec3 to_pos, glm::vec4 to_color, float time_end);
			DrawPointVertex from;
			DrawPointVertex to;
			float time_end;
		};

		Grindstone::GraphicsAPI::VertexBufferLayout vertex_layout_;

		std::vector<DrawableDebugLine> debug_lines_;
		std::vector<DrawableDebugLine> debug_lines_depth_;

		GraphicsAPI::GraphicsPipeline *line_gp_;
		GraphicsAPI::GraphicsPipeline *line_gp_depth_;
	};
};
