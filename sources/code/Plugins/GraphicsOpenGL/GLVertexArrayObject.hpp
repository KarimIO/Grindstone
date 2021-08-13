#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include <Common/Graphics/VertexArrayObject.hpp>
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLVertexArrayObject : public VertexArrayObject {
		public:
			GLuint vertexArrayObject;
			uint32_t vertexBufferCount = 0;
		public:
			GLVertexArrayObject();
			GLVertexArrayObject(CreateInfo& createInfo);
			virtual ~GLVertexArrayObject() override;

			virtual void Bind() override;
			virtual void Unbind() override;
		};
	}
}

#endif