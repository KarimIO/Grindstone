#pragma once

#include "StaticMesh.hpp"

namespace Grindstone {
	class StaticMeshV1Loader {
	public:
		bool processFile(char* filePtr, size_t fileSize, StaticMesh &mesh);
	private:
		GraphicsAPI::VertexBuffer* processVbo();
		GraphicsAPI::IndexBuffer* processIbo();
		void processSubmeshes();
		void processMaterial();
	};
}
