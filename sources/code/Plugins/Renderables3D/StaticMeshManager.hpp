#pragma once

#include "StaticMesh.hpp"

namespace Grindstone {
    class StaticMeshManager {
    public:
        void initialize();
        
        void loadMesh(const char *path);
        void loadMeshImmediate(const char *path);
        void reloadMesh(const char *path);
        void unloadMesh(const char *path);
    private:
        bool loadMeshImpl(const char *path);
        bool processMeshV1(char* offset, size_t file_size);
    private:
        std::vector<StaticMesh> static_meshes_;
        std::map<std::string, uint32_t> static_mesh_type_;

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