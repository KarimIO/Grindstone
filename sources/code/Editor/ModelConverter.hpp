#pragma once

#include <string>
#include <vector>
#include <map>
#include <assimp/Importer.hpp>

class BoundingShape;

class ModelConverter {
public:
	struct Params {
		std::string path;
		std::string skeleton_path;
	};

	ModelConverter(Params params);
private:

	// Definitions
	struct Vertex {
		float positions[3];
		float normal[3];
		float tangent[3];
		float tex_coord[2];
	};

	#define BONES_PER_VERTEX 4

	struct VertexWeights {
		uint16_t	bone_ids[BONES_PER_VERTEX];
		float		bone_weights[BONES_PER_VERTEX];

		VertexWeights() {
			clear();
		}

		void clear() {
			memset(bone_ids, 0, sizeof(bone_ids));
			memset(bone_weights, 0, sizeof(bone_weights));
		}
	};

	struct Mesh {
		uint32_t num_indices = 0;
		uint32_t base_vertex = 0;
		uint32_t base_index = 0;
		uint32_t material_index = UINT32_MAX;
	};

	struct BoneInfo {
		std::string name;
		aiMatrix4x4 matrix;

		BoneInfo(std::string name, aiMatrix4x4 matrix);
	};

	// Methods
private:
	void initMaterials(std::string folder_name, std::string path);
	void initMesh(unsigned int i);
	void loadBones(unsigned int i);
	void addBoneData(uint32_t bone_id, float weight, VertexWeights &v);
	void outputSkeleton();

	// Members
	std::string file_name_;
	aiScene* scene_;

	BoundingShape *bounding_shape_;
	std::vector<std::string> material_names_;
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	std::vector<VertexWeights> vertex_weights_;
	std::vector<Mesh> meshes_;

	std::vector<BoneInfo> bone_info_;
	std::map<std::string, uint16_t> bone_map_;
	uint16_t num_bones_;
	bool create_skeleton_;
};

void parseModelConverterParams(std::string params);
