#include "SGeometryStatic.hpp"
#include "../Core/Engine.hpp"
#include "../FormatCommon/StaticModel.hpp"
#include <fstream>

struct ModelUBO {
	glm::mat4 pvmMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
} ubo;

void MeshStatic::ShadowDraw() {
	for (auto &reference : model->references) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);

		// Cull for shadows seperately
		//if (renderComponent.should_draw) {
			auto entityID = renderComponent.entity_id;
			auto transform = engine.transformSystem.components[entityID];

			engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
			engine.ubo2->Bind();

			engine.graphics_wrapper_->BindVertexArrayObject(model->vertexArrayObject);
			engine.graphics_wrapper_->DrawImmediateIndexed(false, true, BaseVertex, BaseIndex, NumIndices);
		//}
	}
}

void MeshStatic::Draw() {
	for (auto &reference : model->references) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);
		//if (renderComponent.should_draw) {
			auto entityID = renderComponent.entity_id;
			auto transform = engine.transformSystem.components[entityID];

			engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
			engine.ubo2->Bind();

			engine.graphics_wrapper_->BindVertexArrayObject(model->vertexArrayObject);
			engine.graphics_wrapper_->DrawImmediateIndexed(false, true, BaseVertex, BaseIndex, NumIndices);
		//}
	}
}

void MeshStatic::DrawDeferred(CommandBuffer *cmd) {
	cmd->BindBufferObjects(model->vertexBuffer, model->indexBuffer, model->useLargeBuffer);
	cmd->DrawIndexed(BaseVertex, BaseIndex, NumIndices, 1);
}

std::string CModelStatic::getName() {
	return name;
}

void CModelStatic::setName(std::string path) {
	name = path;
}

void SGeometryStatic::LoadGeometry(uint32_t render_id, std::string path) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == path) {
			models[i].references.push_back((uint32_t)render_id);
			return;
		}
	}

	unloadedModelIDs.push_back((unsigned int)(models.size()));
	models.push_back({path, {render_id}});
}

void SGeometryStatic::LoadPreloaded() {
	for (unsigned int unloadedModelID : unloadedModelIDs) {
		CModelStatic *model = &models[unloadedModelID];
		LoadModel(model);
	}
}

void SGeometryStatic::Cull(CCamera *cam) {
	float cam_near = cam->GetNear();
	float cam_far = cam->GetFar();
	float cam_fov = cam->GetFOV() / 2.0f;
	float cam_aspect = cam->GetAspectRatio();
	auto transform_id = engine.entities[cam->entityID].components_[COMPONENT_TRANSFORM];
	auto &transform = engine.transformSystem.components[transform_id];
	glm::vec3 cam_position = transform.GetPosition();
	glm::vec3 forward = transform.GetForward();
	glm::vec3 right = transform.GetRight();
	glm::vec3 up = transform.GetUp();

	float sphere_factor_y = 1.0f / cos(cam_fov);

	cam_fov = tan(cam_fov);
	float angle = atan(cam_fov * cam_aspect);
	float sphere_factor_x = 1.0f / cos(angle);

	unsigned int drawing = 0, total = 0;
	for (auto &model : models) {
		for (auto &reference_id : model.references) {
			total++;
			auto &reference = engine.geometry_system.GetComponent(reference_id);
			auto transform_id = engine.entities[reference.entity_id].components_[COMPONENT_TRANSFORM];
			glm::vec3 obj_position = engine.transformSystem.components[transform_id].GetPosition();
			reference.should_draw = true; // model.bounding->TestCamera(cam_near, cam_far, cam_fov, sphere_factor_x, sphere_factor_y, cam_aspect, obj_position, cam_position, forward, up, right);
			if (reference.should_draw) {
				drawing++;
				for (auto &mesh : model.meshes) {
					mesh.material->IncrementDrawCount();
				}
			}
		}
	}

	// std::cout << "Drawing " << drawing << " / " << total << std::endl;
}

std::vector<CommandBuffer*> SGeometryStatic::GetCommandBuffers() {
	std::vector<CommandBuffer*> buffers;
	buffers.reserve(models.size());
	for (CModelStatic model : models) {
		buffers.push_back(model.commandBuffer);
	}

	return buffers;
}

void SGeometryStatic::LoadModel(CModelStatic *model) {
	std::string path = std::string(model->getName());
	std::ifstream input(path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		std::cerr << "Failed to open file: " << path << "!\n";
		return;
	}

	std::cout << "Model reading from: " << path << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	ModelFormatHeader inFormat;
	void *offset = buffer.data();
	memcpy(&inFormat, offset, sizeof(ModelFormatHeader));
	offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
	switch (inFormat.bounding_type) {
	case BOUNDING_SPHERE:
		model->bounding = new BoundingSphere();
		break;
	case BOUNDING_BOX:
		model->bounding = new BoundingBox();
		break;
	}
	memcpy(model->bounding->GetData(), offset, model->bounding->GetSize());
	model->bounding->Print();
	
	offset = static_cast<char*>(offset) + model->bounding->GetSize();

	model->meshes.resize(inFormat.num_meshes);
	std::vector<Vertex> vertices;
	vertices.resize(inFormat.num_vertices);
	std::vector<unsigned int> indices;
	indices.resize(inFormat.num_indices);

	//offset = static_cast<char*>(offset) + model->bounding->GetSize();
	uint32_t size = inFormat.num_meshes * sizeof(MeshCreateInfo);
	std::vector<MeshCreateInfo> temp_meshes(inFormat.num_meshes);
	memcpy(temp_meshes.data(), offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.num_vertices * sizeof(Vertex);
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.num_indices * sizeof(unsigned int);
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	std::cout << "Loading " << inFormat.num_materials << " materials.\n";
	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.num_materials);

	std::vector<Material *> materials;
	materials.resize(inFormat.num_materials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		// Need to add a non-lazyload material
		std::cout << "Loading Material: " << words << std::endl;
		materialReferences[i] = material_system_->CreateMaterial(geometry_info_, words);
		words = strchr(words, '\0') + 1;
	}

	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		materials[i] = material_system_->GetMaterial(materialReferences[i]);
	}

	input.close();

	VertexBufferCreateInfo vbci;
	vbci.attribute = geometry_info_.vads;
	vbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	vbci.binding = geometry_info_.vbds;
	vbci.bindingCount = geometry_info_.vbds_count;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	/*VertexBufferCreateInfo svbci;
	svbci.attribute = geometry_info_.vads;
	svbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	svbci.binding = geometry_info_.vbds;
	svbci.bindingCount = geometry_info_.vbds_count;
	svbci.content = static_cast<const void *>(vertices.data());
	svbci.count = static_cast<uint32_t>(vertices.size());
	svbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());*/

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	if (graphics_wrapper_->SupportsCommandBuffers()) {
		model->vertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);
	}
	else {
		VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model->vertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->vertexArrayObject = graphics_wrapper_->CreateVertexArrayObject(vaci);
		model->vertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model->vertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->vertexArrayObject->BindResources(vaci);
		model->vertexArrayObject->Unbind();


		/*VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model->shadowVertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->shadowVertexArrayObject = graphics_wrapper_->CreateVertexArrayObject(vaci);
		model->shadowVertexBuffer = graphics_wrapper_->CreateVertexBuffer(svbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model->shadowVertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->shadowVertexArrayObject->BindResources(vaci);
		model->shadowVertexArrayObject->Unbind();*/

	}

	for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model->meshes[i];
		current_mesh.model = model;
		current_mesh.material = materials[temp_mesh.MaterialIndex];
		current_mesh.NumIndices = temp_mesh.NumIndices;
		current_mesh.BaseVertex = temp_mesh.BaseVertex;
		current_mesh.BaseIndex = temp_mesh.BaseIndex;
		current_mesh.material->m_meshes.push_back(reinterpret_cast<Mesh *>(&current_mesh));
	}
}

SGeometryStatic::SGeometryStatic(MaterialManager *material_system, GraphicsWrapper * graphics_wrapper, std::vector<UniformBufferBinding *> ubbs) : graphics_wrapper_(graphics_wrapper), material_system_(material_system) {
	geometry_info_.ubb_count = ubbs.size();
	geometry_info_.ubbs = new UniformBufferBinding *[ubbs.size()];

	for (unsigned int i = 0; i < geometry_info_.ubb_count; i++) {
		geometry_info_.ubbs[i] = ubbs[i];
	}

	geometry_info_.vbds_count = 1;
	geometry_info_.vbds = new VertexBindingDescription();
	geometry_info_.vbds->binding = 0;
	geometry_info_.vbds->elementRate = false;
	geometry_info_.vbds->stride = sizeof(Vertex);

	geometry_info_.vads_count = 4;
	geometry_info_.vads = new VertexAttributeDescription[4];
	geometry_info_.vads[0].binding = 0;
	geometry_info_.vads[0].location = 0;
	geometry_info_.vads[0].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[0].size = 3;
	geometry_info_.vads[0].name = "vertexPosition";
	geometry_info_.vads[0].offset = offsetof(Vertex, positions);
	geometry_info_.vads[0].usage = ATTRIB_POSITION;

	geometry_info_.vads[1].binding = 0;
	geometry_info_.vads[1].location = 1;
	geometry_info_.vads[1].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[1].size = 3;
	geometry_info_.vads[1].name = "vertexNormal";
	geometry_info_.vads[1].offset = offsetof(Vertex, normal);
	geometry_info_.vads[1].usage = ATTRIB_NORMAL;

	geometry_info_.vads[2].binding = 0;
	geometry_info_.vads[2].location = 2;
	geometry_info_.vads[2].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[2].size = 3;
	geometry_info_.vads[2].name = "vertexTangent";
	geometry_info_.vads[2].offset = offsetof(Vertex, tangent);
	geometry_info_.vads[2].usage = ATTRIB_TANGENT;

	geometry_info_.vads[3].binding = 0;
	geometry_info_.vads[3].location = 3;
	geometry_info_.vads[3].format = VERTEX_R32_G32;
	geometry_info_.vads[3].size = 2;
	geometry_info_.vads[3].name = "vertexTexCoord";
	geometry_info_.vads[3].offset = offsetof(Vertex, texCoord);
	geometry_info_.vads[3].usage = ATTRIB_TEXCOORD0;
}

SGeometryStatic::~SGeometryStatic() {
	if (graphics_wrapper_) {
		if (graphics_wrapper_->SupportsCommandBuffers()) {
			for (size_t i = 0; i < models.size(); i++) {
				graphics_wrapper_->DeleteCommandBuffer(models[i].commandBuffer);
				graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
				graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
			}
		}
		else {
			for (size_t i = 0; i < models.size(); i++) {
				graphics_wrapper_->DeleteVertexArrayObject(models[i].vertexArrayObject);
				//graphics_wrapper_->DeleteVertexArrayObject(models[i].shadowVertexArrayObject);
				graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
				graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
			}
		}
	}

	/*for (auto &model : models) {
		delete model.bounding;
	}*/
}