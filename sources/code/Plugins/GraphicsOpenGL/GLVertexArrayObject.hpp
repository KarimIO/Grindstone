#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include <Common/Graphics/VertexArrayObject.hpp>
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLVertexArrayObject : public VertexArrayObject {
		public:
			GLuint vertex_array_object_;
			uint32_t number_buffers_;
		public:
			GLVertexArrayObject();
			GLVertexArrayObject(CreateInfo& createInfo);
			virtual ~GLVertexArrayObject() override;

			virtual void bind() override;
			virtual void unbind() override;
		};
	}
}

#endif