#include <GL/gl3w.h>
#include "GLVertexArrayObject.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLenum vertexFormatToOpenGLFormat(VertexFormat format) {
			switch (format)
			{
			case VertexFormat::Float:
			case VertexFormat::Float2:
			case VertexFormat::Float3:
			case VertexFormat::Float4:
			case VertexFormat::Mat3:
			case VertexFormat::Mat4:	return GL_FLOAT;
			case VertexFormat::Int:
			case VertexFormat::Int2:
			case VertexFormat::Int3:
			case VertexFormat::Int4:	return GL_INT;
			case VertexFormat::UInt:
			case VertexFormat::UInt2:
			case VertexFormat::UInt3:
			case VertexFormat::UInt4:	return GL_UNSIGNED_INT;
			case VertexFormat::Bool:	return GL_BOOL;
			}

			return 0;
		};

		GLVertexArrayObject::GLVertexArrayObject() : number_buffers_(0) {
			glGenVertexArrays(1, &vertex_array_object_);
			glBindVertexArray(vertex_array_object_);
		}

		GLVertexArrayObject::GLVertexArrayObject(VertexArrayObjectCreateInfo createInfo) : number_buffers_(0) {
			glGenVertexArrays(1, &vertex_array_object_);
			glBindVertexArray(vertex_array_object_);

			for (size_t i = 0; i < createInfo.vertex_buffer_count; ++i) {
				GLVertexBuffer* vbo = (GLVertexBuffer *)createInfo.vertex_buffers[i];
				vbo->bind();

				const auto& layout = vbo->getLayout();
				for (int j = 0; j < layout.attribute_count; ++j) {
					VertexAttributeDescription &layout_element = layout.attributes[j];
					GLenum vert_format = vertexFormatToOpenGLFormat(layout_element.format);
					glEnableVertexAttribArray(number_buffers_);
					glVertexAttribPointer(number_buffers_++,
						layout_element.components_count,
						vert_format,
						layout_element.normalized ? GL_TRUE : GL_FALSE,
						layout.stride,
						(const void*)layout_element.offset);
				}
			}

			if (createInfo.index_buffer != nullptr) {
				((GLIndexBuffer*)createInfo.index_buffer)->Bind();
			}

			glBindVertexArray(0);
		}

		GLVertexArrayObject::~GLVertexArrayObject() {
			glDeleteVertexArrays(1, &vertex_array_object_);
		}

		void GLVertexArrayObject::Bind() {
			glBindVertexArray(vertex_array_object_);
		}

		void GLVertexArrayObject::Unbind() {
			glBindVertexArray(0);
		}
	}
}