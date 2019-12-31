#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include "../GraphicsCommon/VertexArrayObject.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLVertexArrayObject : public VertexArrayObject {
		public:
			GLuint vao;
		public:
			GLVertexArrayObject(VertexArrayObjectCreateInfo createInfo);
			~GLVertexArrayObject();

			void Bind();
			void BindResources(VertexArrayObjectCreateInfo createInfo);
			void Unbind();
		};
	}
}

#endif