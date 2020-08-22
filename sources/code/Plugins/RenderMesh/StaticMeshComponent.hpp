#pragma once

#include "StaticMesh.hpp"

namespace Grindstone {
    struct StaticMeshComponent {
        const char *path_;
        StaticMesh *static_mesh_ = nullptr;
    };
}