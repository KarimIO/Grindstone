#include "pch.hpp"
#include "StaticMeshManager.hpp"
#include <Plugins/Materials/MaterialManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <glm/glm.hpp>
#include <iostream>

namespace Grindstone {
	StaticMeshManager::StaticMeshManager() {
		GraphicsAPI::VertexBufferLayout vbd({
			{ GraphicsAPI::VertexFormat::Float3, "vPosition",	false, GraphicsAPI::AttributeUsage::Position },
			{ GraphicsAPI::VertexFormat::Float3, "vNormal",		false, GraphicsAPI::AttributeUsage::Normal },
			{ GraphicsAPI::VertexFormat::Float3, "vTangent",	false, GraphicsAPI::AttributeUsage::Tangent },
			{ GraphicsAPI::VertexFormat::Float4, "vColor",		false, GraphicsAPI::AttributeUsage::Color },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord0",	false, GraphicsAPI::AttributeUsage::TexCoord0 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord1",	false, GraphicsAPI::AttributeUsage::TexCoord1 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord2",	false, GraphicsAPI::AttributeUsage::TexCoord2 },
			{ GraphicsAPI::VertexFormat::Float4, "vTexCoord3",	false, GraphicsAPI::AttributeUsage::TexCoord3 },
			{ GraphicsAPI::VertexFormat::Float4, "vBlendWeights",false, GraphicsAPI::AttributeUsage::BlendWeights },
			{ GraphicsAPI::VertexFormat::Float4, "vBlendIndices",false, GraphicsAPI::AttributeUsage::BlendIndices }
		});
	}

	bool StaticMeshManager::loadMeshImpl(const char* path) {
		Renderer::MaterialManager *material_manager;

		std::string fullpath = std::string("../assets/") + path;
		std::ifstream input(fullpath, std::ios::ate | std::ios::binary);

		if (!input.is_open()) {
			//GRIND_ERROR("Failed to open file: {0}!", path);
			return false;
		}

		size_t file_size = (size_t)input.tellg();
		std::vector<char> buffer(file_size);

		input.seekg(0);
		input.read(buffer.data(), file_size);

		if (file_size < 4) {
			// GRIND_ERROR("File is too small!");
			return false;
		}

		if (buffer[0] != 'G' || buffer[1] != 'M' || buffer[2] != 'F') {
			// GRIND_ERROR("Failed to open file: {0}!", path);
			return false;
		}

		char* offset = buffer.data() + 3;

		uint8_t version;
		memcpy(&version, offset, sizeof(uint8_t));

		bool result = true;
		switch (version) {
		case 1:
			result = processMeshV1(offset, file_size);
		default:
			std::cout << "Invalid version number!\r\n";
			result = false;
		}

		input.close();
		return result;
	};


	bool StaticMeshManager::processMeshV1(char* offset, size_t file_size) {
		GraphicsAPI::Core* graphics_core;
		MaterialManager* material_manager;
		StaticMesh mesh;

		if (file_size < sizeof(ModelFormatHeader::V1) + 4) {
			// GRIND_ERROR("File is too small!");
			return false;
		}

		ModelFormatHeader::V1 mesh_header;
		memcpy(&mesh_header, offset, sizeof(ModelFormatHeader::V1));
		offset = offset + sizeof(ModelFormatHeader::V1);


		if (file_size < mesh_header.total_file_size_) {
			// GRIND_ERROR("File is too small!");
			return false;
		}
		
		memcpy(&mesh.bounding_, offset, sizeof(mesh.bounding_));
		offset = offset + sizeof(mesh.bounding_);

		std::vector<char> vertex_data;
		vertex_data.resize(mesh_header.vertices_size_);
		std::vector<unsigned int> indices;

		char* target_offset = vertex_data.data();
		size_t part_size3 = mesh_header.num_vertices_ * sizeof(glm::vec3);
		size_t part_size4 = mesh_header.num_vertices_ * sizeof(glm::vec4);

		bool set_if_null = true;

		// ===================
		// Vertex Buffer
		// ===================
		GraphicsAPI::VertexBuffer* vbo;
		{
			for (int i = 1; i <= 256; i *= 2) {
				// Only use vec3 size on Normal
				uint32_t size = (i == 2) ? part_size3 : part_size4;

				if (mesh_header.vertex_flags_ & i) {
					memcpy(target_offset, offset, size);
					offset += size;
					target_offset += size;
				}
				else if (set_if_null) {
					memset(target_offset, 0, size);
					target_offset += size;
				}
			}

			Grindstone::GraphicsAPI::VertexBuffer::CreateInfo vbci;
			vbci.layout = &vertex_layout_;
			vbci.content = static_cast<const void*>(vertex_data.data());
			vbci.count = mesh_header.vertices_size_ / sizeof(float);
			vbci.size = mesh_header.vertices_size_;
			vbo = graphics_core->createVertexBuffer(vbci);
		}

		// ===================
		// Index Buffer
		// ===================
		GraphicsAPI::IndexBuffer* ibo;
		{
			// Copy Indices
			indices.resize(mesh_header.num_indices_);
			size_t size_indices = mesh_header.num_indices_ * sizeof(unsigned int);
			memcpy(indices.data(), offset, size_indices);
			offset = static_cast<char*>(offset) + size_indices;

			Grindstone::GraphicsAPI::IndexBuffer::CreateInfo ibci;
			ibci.content = static_cast<const void*>(indices.data());
			ibci.count = static_cast<uint32_t>(indices.size());
			ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());
			ibo = graphics_core->createIndexBuffer(ibci);
		}

		// Copy Meshes
		mesh.submeshes_.resize(mesh_header.num_meshes_);
		auto& submeshes = mesh.submeshes_;
		uint32_t size = mesh_header.num_meshes_ * sizeof(StaticSubmesh);
		memcpy(submeshes.data(), offset, size);
		offset = static_cast<char*>(offset) + size;

		std::vector<MaterialReference> material_references;
		material_references.resize(mesh_header.num_materials_);

		std::vector<Material *> materials;
		materials.resize(mesh_header.num_materials_);
		char *material_path = (char *)offset;
		for (unsigned int i = 0; i < mesh_header.num_materials_; i++) {
			material_manager->loadMaterial(material_path, materials[i]);
			material_path = strchr(material_path, '\0') + 1;
		}

		if (graphics_core->shouldUseImmediateMode()) {
			Grindstone::GraphicsAPI::VertexArrayObject::CreateInfo vaci;
			vaci.vertex_buffers = &vbo;
			vaci.vertex_buffer_count = 1;
			vaci.index_buffer = ibo;
			mesh.vao_ = graphics_core->createVertexArrayObject(vaci);
		}

	}
}