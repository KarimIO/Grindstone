#ifndef _STATIC_MODEL_H
#define _STATIC_MODEL_H

#include "Bounding.hpp"
#include <stdint.h>

struct ModelFormatHeader {
	uint32_t version;
	uint32_t num_meshes;
	uint64_t num_vertices;
	uint64_t num_indices;
	uint32_t num_materials;
	uint16_t num_bones;
	bool large_index;
	BoundingType bounding_type;
};

#endif