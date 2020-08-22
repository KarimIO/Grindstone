#pragma once

#include "StaticMeshManager.hpp"

namespace Grindstone {
    void StaticMeshManager::addMesh(const char *path) {
        /*Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
        MaterialManager *material_system = engine.getMaterialManager();

        std::string path = std::string("../assets/" + model.path_);
        std::ifstream input(path, std::ios::ate | std::ios::binary);

        if (!input.is_open()) {
            GRIND_ERROR("Failed to open file: {0}!", path);
            return false;
        }

        GRIND_LOG("Model reading from: {0}", path );

        size_t fileSize = (size_t)input.tellg();
        std::vector<char> buffer(fileSize);

        input.seekg(0);
        input.read(buffer.data(), fileSize);

        if (buffer[0] != 'G' || buffer[1] != 'M' || buffer[2] != 'F') {
            GRIND_ERROR("Failed to open file: {0}!", path);
            return false;
        }

        ModelFormatHeader inFormat;
        void *offset = buffer.data() + 3;
        memcpy(&inFormat, offset, sizeof(ModelFormatHeader));
        offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
        switch (inFormat.bounding_type) {
        case BOUNDING_SPHERE:
            model.bounding = new BoundingSphere();
            break;
        case BOUNDING_BOX:
            model.bounding = new BoundingBox();
            break;
        }
        memcpy(model.bounding->GetData(), offset, model.bounding->GetSize());
        model.bounding->Print();

        offset = static_cast<char*>(offset) + model.bounding->GetSize();

        std::vector<Vertex> vertices;
        std::vector<VertexWeights> vertex_weights;
        std::vector<unsigned int> indices;

        std::vector<MeshCreateInfo> temp_meshes(inFormat.num_meshes);
        model.meshes.resize(inFormat.num_meshes);
        vertices.resize(inFormat.num_vertices);
        vertex_weights.resize(inFormat.num_vertices);
        indices.resize(inFormat.num_indices);

        // Copy Meshes
        uint32_t size = inFormat.num_meshes * sizeof(MeshCreateInfo);
        memcpy(temp_meshes.data(), offset, size);
        offset = static_cast<char*>(offset) + size;

        // Copy Vertices
        uint64_t size64 = inFormat.num_vertices * sizeof(Vertex);
        memcpy(vertices.data(), offset, size64);
        offset = static_cast<char*>(offset) + size64;

        // Copy Vertex Weights
        if (inFormat.has_bones) {
            size64 = inFormat.num_vertices * sizeof(VertexWeights);
            memcpy(vertex_weights.data(), offset, size64);
            offset = static_cast<char*>(offset) + size64;
        }

        // Copy Indices
        size64 = inFormat.num_indices * sizeof(unsigned int);
        memcpy(indices.data(), offset, size64);
        offset = static_cast<char*>(offset) + size64;

        std::vector<MaterialReference> materialReferences;
        materialReferences.resize(inFormat.num_materials);

        std::vector<Material *> materials;
        materials.resize(inFormat.num_materials);
        char *words = (char *)offset;
        for (unsigned int i = 0; i < inFormat.num_materials; i++) {
            // Need to add a non-lazyload material
            materialReferences[i] = material_system->loadMaterial(geometry_info_, words);
            if (materialReferences[i].pipelineReference.pipeline_type == TYPE_MISSING) {
                materialReferences[i] = empty_material;
            }
            words = strchr(words, '\0') + 1;
        }

        for (unsigned int i = 0; i < inFormat.num_materials; i++) {
            materials[i] = material_system->getMaterial(materialReferences[i]);
        }

        input.close();

        Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
        vbci.layout = &vertex_layout_;
        vbci.content = static_cast<const void *>(vertices.data());
        vbci.count = static_cast<uint32_t>(vertices.size());
        vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

        Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
        ibci.content = static_cast<const void *>(indices.data());
        ibci.count = static_cast<uint32_t>(indices.size());
        ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

        model.vertex_buffer = graphics_wrapper->createVertexBuffer(vbci);
        model.index_buffer = graphics_wrapper->createIndexBuffer(ibci);

        if (graphics_wrapper->shouldUseImmediateMode()) {
            Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
            vaci.vertex_buffers = &model.vertex_buffer;
            vaci.vertex_buffer_count = 1;
            vaci.index_buffer = model.index_buffer;
            model.vertex_array_object = graphics_wrapper->createVertexArrayObject(vaci);
            
        }

        for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
            // Use the temporarily uint32_t material as an ID for the actual material.
            MeshCreateInfo &temp_mesh = temp_meshes[i];
            MeshStatic &current_mesh = model.meshes[i];
            current_mesh.model_reference = model.handle_;
            current_mesh.material_reference = materialReferences[temp_mesh.material_index];
            current_mesh.num_indices = temp_mesh.num_indices;
            current_mesh.base_vertex = temp_mesh.base_vertex;
            current_mesh.base_index = temp_mesh.base_index;
            //current_mesh.material->m_meshes.push_back(reinterpret_cast<MeshStatic *>(&current_mesh));
            materials[temp_mesh.material_index]->m_meshes.push_back(&current_mesh);
        }*/

    }
}