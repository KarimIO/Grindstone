#include "ModelConverter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include "MaterialCreator.hpp"
#include "Utilities.hpp"
#include "AnimationConverter.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// File Format:
// grindstone_anim
// length
// for each bone:
//		num_keyframes
//		for each keyframe:
//			position rotation scale time

void AnimationConverter::getAnimationLength() {
	// To get anim length, get the time of the last keyframe of the 0th channel, of the 0th animation
	unsigned int numPosKeys = scene_->mAnimations[0]->mChannels[0]->mNumPositionKeys;
	animation_length_ = scene_->mAnimations[0]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;
}

void AnimationConverter::processFrames() {
	nodes_.resize(bone_info_.size());

	for (unsigned int i = 0; i < scene_->mRootNode->mNumChildren; ++i) {
		traverseNode(scene_->mRootNode->mChildren[i]);
	}
}

void AnimationConverter::processScene() {
	// Get total timegetAnimationLength();
	getAnimationLength();

	// Load bone info
	processFrames();

	std::ofstream output(params_.output_path, std::ios::binary);
	// Output File MetaData
	output.write("GAF", 3);
	// Output animation length
	output << animation_length_;
	// Output total number of bones
	output << nodes_.size();
	// For each bone
	for (auto &node : nodes_) {
		// Output the number of keyframes
		output << node.keyframes_.size();
		// Output all anim data for bone
		output.write(reinterpret_cast<const char*> (node.keyframes_.data()), node.keyframes_.size() * sizeof(Keyframe));
	}

	output.close();
}

const aiNodeAnim *AnimationConverter::findNodeAnim(const aiAnimation *animation, std::string bone_name) {
	for (unsigned int i = 0; i < animation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = animation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == bone_name) {
			return pNodeAnim;
		}
	}
}

void AnimationConverter::traverseNode(const aiNode * node) {
	if (node->mName.length > 0) {
		std::string bone_name(node->mName.data);
		std::cout << "**> " << bone_name << "\n";

		// Handle bone name.
		bool found = false;
		for (int i = 0; i < bone_info_.size(); ++i) {
			if (bone_info_[i].bone_name == bone_name) {
				found = true;
				std::vector<Keyframe> &keyframes = nodes_[i].keyframes_;

				// Handle bone keyframes...
				const aiAnimation* animation = scene_->mAnimations[0];

				// TODO: Consider handling this?
				aiMatrix4x4 tp1 = node->mTransformation;
				const aiNodeAnim* node_anim = findNodeAnim(animation, bone_name);

				// TODO: Handle this exception?
				if (!(node_anim->mNumPositionKeys == node_anim->mNumRotationKeys && node_anim->mNumRotationKeys == node_anim->mNumScalingKeys)) {
					throw std::runtime_error("Uneven number of keys for: " + bone_name + "!");
				}

				// Process all keys for bone
				for (int j = 0; j < node_anim->mNumPositionKeys; ++j) {
					aiVectorKey posn_key	= node_anim->mPositionKeys[j];
					aiVectorKey scale_key	= node_anim->mScalingKeys[j];
					aiQuatKey rotate_key	= node_anim->mRotationKeys[j];

					keyframes.emplace_back(Keyframe());
					auto &key = keyframes.back();
					key.position_.x	= posn_key.mValue.x;
					key.position_.y	= posn_key.mValue.y;
					key.position_.z	= posn_key.mValue.z;

					key.scale_.x	= scale_key.mValue.x;
					key.scale_.y	= scale_key.mValue.y;
					key.scale_.z	= scale_key.mValue.z;

					key.rotation_.x	= rotate_key.mValue.x;
					key.rotation_.y	= rotate_key.mValue.y;
					key.rotation_.z	= rotate_key.mValue.z;
					key.rotation_.w	= rotate_key.mValue.w;

					key.time_		= posn_key.mTime;
				}
			}
		}

		// Unhandled bone
		/*if (!found) {
			throw std::runtime_error("Invalid bone in skeleton: " + bone_name + "!");
		}*/
	}

	// Traverse every child
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		traverseNode(node->mChildren[i]);
	}
}

AnimationConverter::AnimationConverter(Params params) : params_(params) {
	auto t_start = std::chrono::high_resolution_clock::now();

	// Set path info
	switchSlashes(params.path);
	std::string file_name = extractFilename(params.path);
	if (params_.output_path == "")
		params_.output_path = "../assets/animations/" + file_name + ".gaf";
	std::cout << "Loading animation: " << params.path << ".\n";

	// Load skeleton names
	GrindstoneAssetCommon::loadSkeleton(params.skeleton_path, global_inverse_, bone_info_);

	// Load scene
	Assimp::Importer importer;
	scene_ = (aiScene *)importer.ReadFile(params.path, 0);

	// If the import failed, report it
	if (!scene_) {
		throw std::runtime_error(importer.GetErrorString());
	}

	// If there are no animations, ignore this.
	if (!scene_->HasAnimations()) {
		throw std::runtime_error("No animations!");
	}

	processScene();

	// Print final info.
	auto t_end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration<double, std::milli>(t_end - std::chrono::high_resolution_clock::now()).count() << " ms\n";
	std::cout << "Animation outputted to: " << params_.output_path << "!\n";
}

AnimationConverter::AnimationConverter(aiScene *scene, std::string skeleton_path) : scene_(scene) {
	auto t_start = std::chrono::high_resolution_clock::now();

	// TODO: process Path info

	// Load skeleton names
	GrindstoneAssetCommon::loadSkeleton(skeleton_path, global_inverse_, bone_info_);
	processScene();

	// Print final info.
	auto t_end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration<double, std::milli>(t_end - std::chrono::high_resolution_clock::now()).count() << " ms\n";
	std::cout << "Animation outputted to: " << params_.output_path << "!\n";
}

void parseAnimationConverterParams(std::string args) {
	AnimationConverter::Params params;
	params.path = args.substr(5);
	params.output_path = "";
	std::cout << "Please input skeleton path (relative to ../assets/models): ";
	std::cin >> params.skeleton_path;

	AnimationConverter m(params);
}