#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include "../GraphicsCommon/VertexArrayObject.hpp"
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
			GLVertexArrayObject(VertexArrayObjectCreateInfo createInfo);
			~GLVertexArrayObject();

			virtual void Bind() override;
			virtual void Unbind() override;
		};
	}
}

#endif