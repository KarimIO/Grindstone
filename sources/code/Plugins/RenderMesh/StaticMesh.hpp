#pragma once

#include <Common/Graphics/VertexArrayObject.hpp>

namespace Grindstone {
    struct StaticSubmesh {
        uint32_t num_indices_ = 0;
        uint32_t base_vertex_ = 0;
        uint32_t base_index_ = 0;
        // Material Reference
    };

    struct StaticMesh {
        GraphicsAPI::VertexArrayObject* vao_ = nullptr;
        // Submeshes
    };
}