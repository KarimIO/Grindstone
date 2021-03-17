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
	private:
		struct ModelFormatHeader {
			struct V1 {
				uint64_t total_file_size_;
				uint32_t num_meshes_;
				uint64_t num_vertices_;
				uint64_t vertices_size_;
				uint64_t num_indices_;
				uint32_t num_materials_;
				uint8_t vertex_flags_;
				bool has_bones_;
				enum class IndexSize {
					Bit16,
					Bit32
				};
			};
		};

	};
}
