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

	bool StaticMeshManager::processFile(char* filePtr, size_t fileSize, StaticMesh &mesh) {
		GraphicsAPI::Core* graphics_core;
		MaterialManager* material_manager;

		if (fileSize < sizeof(ModelFormatHeader::V1) + 4) {
			// GRIND_ERROR("File is too small!");
			return false;
		}

		ModelFormatHeader::V1 mesh_header;
		memcpy(&mesh_header, filePtr, sizeof(ModelFormatHeader::V1));
		filePtr = filePtr + sizeof(ModelFormatHeader::V1);


		if (fileSize < mesh_header.total_fileSize_) {
			// GRIND_ERROR("File is too small!");
			return false;
		}
		
		memcpy(&mesh.bounding_, filePtr, sizeof(mesh.bounding_));
		filePtr = filePtr + sizeof(mesh.bounding_);

		char* target_filePtr = vertex_data.data();
		size_t part_size3 = mesh_header.num_vertices_ * sizeof(glm::vec3);
		size_t part_size4 = mesh_header.num_vertices_ * sizeof(glm::vec4);

		bool set_if_null = true;

		GraphicsAPI::VertexBuffer* vbo = processVbo();
		GraphicsAPI::IndexBuffer* ibo = processIbo();
		processSubmeshes();
		processMaterials();

		if (graphics_core->shouldUseImmediateMode()) {
			Grindstone::GraphicsAPI::VertexArrayObject::CreateInfo vaci;
			vaci.vertex_buffers = &vbo;
			vaci.vertex_buffer_count = 1;
			vaci.index_buffer = ibo;
			mesh.vao_ = graphics_core->createVertexArrayObject(vaci);
		}
	}

	GraphicsAPI::VertexBuffer* StaticMeshManager::processVbo() {
		std::vector<char> vertex_data;
		vertex_data.resize(mesh_header.vertices_size_);

		for (int i = 1; i <= 256; i *= 2) {
			// Only use vec3 size on Normal
			uint32_t size = (i == 2) ? part_size3 : part_size4;

			if (mesh_header.vertex_flags_ & i) {
				memcpy(target_filePtr, filePtr, size);
				filePtr += size;
				target_filePtr += size;
			}
			else if (set_if_null) {
				memset(target_filePtr, 0, size);
				target_filePtr += size;
			}
		}

		Grindstone::GraphicsAPI::VertexBuffer::CreateInfo vbci;
		vbci.layout = &vertex_layout_;
		vbci.content = static_cast<const void*>(vertex_data.data());
		vbci.count = mesh_header.vertices_size_ / sizeof(float);
		vbci.size = mesh_header.vertices_size_;
		return graphics_core->createVertexBuffer(vbci);
	}

	GraphicsAPI::IndexBuffer* StaticMeshManager::processIbo(
		ModelFormatHeader::V1& meshHeaderV1
	) {
		std::vector<unsigned int> indices;
		indices.resize(meshHeaderV1.num_indices_);
		size_t size_indices = meshHeaderV1.num_indices_ * sizeof(unsigned int);
		memcpy(indices.data(), filePtr, size_indices);
		filePtr = static_cast<char*>(filePtr) + size_indices;

		Grindstone::GraphicsAPI::IndexBuffer::CreateInfo ibci;
		ibci.content = static_cast<const void*>(indices.data());
		ibci.count = static_cast<uint32_t>(indices.size());
		ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());
		return graphics_core->createIndexBuffer(ibci);
	}

	void StaticMeshManager::processSubmeshes() {
		mesh.submeshes_.resize(mesh_header.num_meshes_);
		auto& submeshes = mesh.submeshes_;
		uint32_t size = mesh_header.num_meshes_ * sizeof(StaticSubmesh);
		memcpy(submeshes.data(), filePtr, size);
		filePtr = static_cast<char*>(filePtr) + size;
	}

	void StaticMeshManager::processMaterials() {
		std::vector<MaterialReference> material_references;
		material_references.resize(mesh_header.num_materials_);

		std::vector<Material*> materials;
		materials.resize(mesh_header.num_materials_);
		char* material_path = (char*)filePtr;
		for (unsigned int i = 0; i < mesh_header.num_materials_; i++) {
			material_manager->loadMaterial(material_path, materials[i]);
			material_path = strchr(material_path, '\0') + 1;
		}
	}
}