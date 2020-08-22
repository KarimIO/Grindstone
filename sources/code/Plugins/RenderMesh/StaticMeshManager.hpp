#pragma once

namespace Grindstone {
    class StaticMeshManager {
    public:
        void initialize();
        void addMesh(const char *path);
    };
}