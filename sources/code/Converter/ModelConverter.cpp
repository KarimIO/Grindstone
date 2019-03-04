#include "ModelConverter.hpp"
#include "MaterialCreator.hpp"
#include "ImageConverter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <chrono>

#include "Utilities.hpp"
#include "ModelConverter.hpp"

#include "../FormatCommon/Bounding.hpp"
#include "../FormatCommon/StaticModel.hpp"

void ModelConverter::addBoneData(uint32_t bone_id, float weight, VertexWeights &v) {
	const unsigned int n = BONES_PER_VERTEX;

	for (uint16_t i = 0; i < n; i++) {
		if (v.bone_weights[i] == 0.0f) {
			v.bone_ids[i] = bone_id;
			v.bone_weights[i] = weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

void ModelConverter::initMaterials(std::string folder_name, std::string path) {
	uint32_t num_materials = scene_->mNumMaterials;
	aiMaterial **materials = scene_->mMaterials;

	std::string finalDir = path;
	finalDir = finalDir.substr(0, finalDir.find_last_of('/') + 1);

	std::string outPath = "../assets/materials/" + folder_name + "/";
	if (!CreateFolder(outPath.c_str())) {
		outPath = "../assets/materials/";
	}

	aiMaterial *pMaterial;
	aiString Path;
	for (uint32_t i = 0; i < num_materials; i++) {
		StandardMaterialCreateInfo newMat;
		newMat.albedoPath = "";
		newMat.normalPath = "";
		newMat.specularPath = "";
		newMat.roughnessPath = "";
		pMaterial = materials[i];

		aiString name;
		pMaterial->Get(AI_MATKEY_NAME, name);

		if (strcmp(name.C_Str(), "") == 0) {
			name = "Material_" + std::to_string(i);
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				switchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				name = SwapExtension(name, "dds");
				std::string finaloutpath = outPath + name;
				ConvertTexture(finalDir + FullPath, false, finaloutpath, C_BC1);
				newMat.albedoPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0) {
			if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				switchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				name = SwapExtension(name, "dds");
				std::string finaloutpath = outPath + name;
				ConvertTexture(finalDir + FullPath, false, finaloutpath, C_BC1);
				newMat.normalPath = name;
			}
		}
		else if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
			if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				switchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				name = SwapExtension(name, "dds");
				std::string finaloutpath = outPath + name;
				ConvertTexture(finalDir + FullPath, false, finaloutpath, C_BC1);
				newMat.normalPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0) {
			if (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				switchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				name = SwapExtension(name, "dds");
				std::string finaloutpath = outPath + name;
				ConvertTexture(finalDir + FullPath, false, finaloutpath, C_BC1);
				newMat.specularPath = name;
			}
		}

		if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Path.data;
				switchSlashes(FullPath);
				std::string name = FullPath.substr(FullPath.find_last_of("/") + 1);
				name = SwapExtension(name, "dds");
				std::string finaloutpath = outPath + name;
				ConvertTexture(finalDir + FullPath, false, finaloutpath, C_BC1);
				newMat.roughnessPath = name;
			}
		}

		aiColor4D diffuse_color;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color))
			memcpy(&newMat.albedoColor, &diffuse_color, sizeof(glm::vec4));

		aiColor4D metalness;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &metalness))
			newMat.metalness = metalness.r;

		aiColor4D roughness;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_AMBIENT, &roughness))
			newMat.roughness = roughness.r;

		std::string sanname = sanitizeFileName(name.C_Str());

		std::string outMat = outPath + sanname + ".gmat";
		material_names_[i] = outMat;

		std::cout << "\tOutputting material: " << outMat << "\n";

		//std::string shader = skeletalMaterials ? "../shaders/skeletal" : "../shaders/standard";
		CreateStandardMaterial(newMat, outMat);
	}
}

void ModelConverter::initMesh(unsigned int i) {
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	auto &mesh = scene_->mMeshes[i];

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(mesh->mVertices[i]);
		const aiVector3D* pNormal = &(mesh->mNormals[i]);
		const aiVector3D* pTangent = &(mesh->mTangents[i]);
		const aiVector3D* pTexCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : &Zero3D;

		vertices_.push_back({
			{ pPos->x, pPos->y, pPos->z },
			{ pNormal->x, pNormal->y, pNormal->z },
			{ -pTangent->x, -pTangent->y, -pTangent->z },
			{ pTexCoord->x, pTexCoord->y }
		});

		const float pos[3]{ pPos->x, pPos->y, pPos->z };
		bounding_shape_->TestBounding(pos);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& Face = mesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices_.push_back(Face.mIndices[0]);
		indices_.push_back(Face.mIndices[1]);
		indices_.push_back(Face.mIndices[2]);
	}

	loadBones(i);
}

void ModelConverter::loadBones(unsigned int i) {
	const aiMesh *ai_mesh = scene_->mMeshes[i];
	Mesh *g_mesh = &meshes_[i];

	for (unsigned int i = 0; i < ai_mesh->mNumBones; ++i) {
		unsigned int bone_index = 0;
		aiBone *bone = ai_mesh->mBones[i];

		std::string bone_name = bone->mName.C_Str();

		// If we can't find the name of the bone, create a new one
		auto b = bone_map_.find(bone_name);
		if (b == bone_map_.end()) {
			bone_index = num_bones_++;

			bone_map_[bone_name] = bone_index;
			bone_info_.emplace_back(bone_name, (bone->mOffsetMatrix));
		}
		else {
			bone_index = b->second;
		}

		for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
			aiVertexWeight &vert_weight = bone->mWeights[j];
			unsigned int vertex_id = g_mesh->base_vertex + vert_weight.mVertexId;
			float weight = vert_weight.mWeight;
			addBoneData(bone_index, weight, vertex_weights_[vertex_id]);
		}
	}
}

/*
Skeleton File Structure: 
 - GSF
 - NumBones
 - Bones:
	- BoneName
	- BoneMatrix
 - Heirarchy:
	- NumChildren
	- Children
*/

void ModelConverter::outputSkeleton() {
	std::string skeleton_output_path = "../assets/models/" + file_name_ + ".gsf";
	std::cout << "Outputting skeleton with " << bone_info_.size() << " bones to: " << skeleton_output_path << "\n";

	std::ofstream output(skeleton_output_path, std::ios::binary);

	//  - Output File MetaData
	output.write("GSF", 3);

	uint16_t num_bones = bone_info_.size();
	//  - Output number of bones
	output.write((char *)&num_bones, sizeof(uint16_t));

	//  - Output File MetaData
	for (auto &b : bone_info_) {
		output << b.name << '\0';
		output.write((const char *)&b.matrix, sizeof(aiMatrix4x4));
	}
	output.close();
}

// TODO: Allow for optional existing Skeleton to prevent creation of extra skeletons
ModelConverter::ModelConverter(Params params) : num_bones_(0) {
	auto t_start = std::chrono::high_resolution_clock::now();

	switchSlashes(params.path);
	file_name_ = extractFilename(params.path);
	std::string model_output_path("../assets/models/" + file_name_ + ".gmf");
	std::cout << "Loading model: " << params.path << ".\n";

	Assimp::Importer importer;
	scene_ = (aiScene *)importer.ReadFile(params.path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals);
	// | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_PreTransformVertices

	// If the import failed, report it
	if (!scene_) {
		throw std::runtime_error(importer.GetErrorString());
	}

	create_skeleton_ = (params.skeleton_path == "");
	if (!create_skeleton_) {
		// Load the preexisting skeleton path
		// loadBones(params.skeleton_path) => bone_map = contents;
	}


	std::cout << "Model Loaded! Parsing.\n";

	unsigned int num_vertices = 0;
	unsigned int num_indices = 0;

	// Prepare Mesh Info
	meshes_.resize(scene_->mNumMeshes);
	for (unsigned int i = 0; i < scene_->mNumMeshes; i++) {
		meshes_[i].num_indices = scene_->mMeshes[i]->mNumFaces * 3;
		meshes_[i].base_vertex = num_vertices;
		meshes_[i].base_index = num_indices;
		meshes_[i].material_index = scene_->mMeshes[i]->mMaterialIndex;

		num_vertices += scene_->mMeshes[i]->mNumVertices;
		num_indices += meshes_[i].num_indices;
	}

	vertices_.reserve(num_vertices);
	vertex_weights_.resize(num_vertices);
	indices_.reserve(num_indices);

	// Prepare materials
	material_names_.resize(scene_->mNumMaterials);
	if (scene_->HasMaterials()) {
		initMaterials(file_name_, params.path);
	}

	// Prepare Vertex Info and Bounding Box
	bounding_shape_ = new BoundingBox();
	for (unsigned int i = 0; i < scene_->mNumMeshes; i++) {
		initMesh(i);
	}

	importer.FreeScene();

	std::cout << "Model Parsed! Outputting.\n";

	// Prepare Model Header
	ModelFormatHeader outFormat;
	outFormat.large_index = false;
	outFormat.num_vertices = static_cast<uint64_t>(vertices_.size());
	outFormat.num_indices = static_cast<uint64_t>(indices_.size());
	outFormat.num_meshes = static_cast<uint32_t>(meshes_.size());
	outFormat.num_materials = static_cast<uint32_t>(material_names_.size());
	outFormat.has_bones = num_bones_ > 0;
	outFormat.bounding_type = BOUNDING_BOX;

	if (create_skeleton_ && outFormat.has_bones)
		outputSkeleton();

	std::ofstream output(model_output_path, std::ios::binary);

	//  - Output File MetaData
	output.write("GMF", 3);

	//	- Output Header
	output.write(reinterpret_cast<const char*> (&outFormat), sizeof(ModelFormatHeader));

	//	- Output Bounding Size
	output.write(reinterpret_cast<const char*> (bounding_shape_->GetData()), bounding_shape_->GetSize());
	bounding_shape_->Print();

	//	- Output Meshes
	std::cout << "Outputting mesh info for " << meshes_.size() << " meshes.\n";
	output.write(reinterpret_cast<const char*> (meshes_.data()), meshes_.size() * sizeof(Mesh));

	//	- Ouput Vertices
	std::cout << "Outputting " << vertices_.size() << " vertices.\n";
	output.write(reinterpret_cast<const char*> (vertices_.data()), vertices_.size() * sizeof(Vertex));

	// - Ouput Bone Info
	if (outFormat.has_bones) {
		std::cout << "Outputting bone weight info.\n";
		output.write(reinterpret_cast<const char*> (vertex_weights_.data()), vertex_weights_.size() * sizeof(VertexWeights));
	}

	// TODO: Output all TexCoords here

	// - Output Indices
	std::cout << "Outputting " << indices_.size() << " indices.\n";

	// - Output Materials
	output.write(reinterpret_cast<const char*> (indices_.data()), indices_.size() * sizeof(uint32_t));
	for (const auto &matName : material_names_) {
		output << matName << '\0';
	}

	output.close();

	// Print final info.
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " ms\n";
	std::cout << "Model Outputted to: " << model_output_path << "!\n";
}

void parseModelConverterParams(std::string args) {
	ModelConverter::Params params;
	params.path = args.substr(4);
	params.skeleton_path = "";

	ModelConverter m(params);
}

ModelConverter::BoneInfo::BoneInfo(std::string n, aiMatrix4x4 m) : name(n), matrix(m) {}
