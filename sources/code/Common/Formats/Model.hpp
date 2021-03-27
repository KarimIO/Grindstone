#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace Formats {
		namespace Model {
			enum class IndexSize {
				Bit16,
				Bit32
			};

			namespace Header {
				struct V1 {
					uint64_t total_file_size_;
					uint32_t num_meshes_;
					uint64_t num_vertices_;
					uint64_t vertices_size_;
					uint64_t num_indices_;
					uint32_t num_materials_;
					uint8_t vertex_flags_;
					bool has_bones_;
				};
			};
		}
	}
}