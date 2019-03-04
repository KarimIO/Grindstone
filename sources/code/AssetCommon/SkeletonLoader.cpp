#include "SkeletonLoader.hpp"
#include <fstream>
#include <iostream>

void GrindstoneAssetCommon::loadSkeleton(std::string path, glm::mat4 global_inverse, std::vector<BoneInfo> bone_names) {
	// Read all bone names here.
	std::ifstream input("../assets/models/" + path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		throw std::runtime_error("Failed to open file: " + path + "!");
	}

	std::cout << "Model reading from: " << path << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);
	
	auto bufferpos = buffer.data();

	if (buffer[0] != 'G' && buffer[1] != 'S' || buffer[2] != 'F') {
		throw std::runtime_error("Invalid File: Doesn't start with GSF");
	}
	
	uint16_t num_bones = (uint16_t)bufferpos[3];
	bone_names.resize(num_bones);

	bufferpos += 3 * sizeof(char) + sizeof(uint16_t);
	
	// Copy global inverse
	memcpy((void *)&global_inverse, bufferpos, sizeof(glm::mat4));

	// Go to bones
	bufferpos += sizeof(float) * 16;

	for (size_t i = 0; i < num_bones; ++i) {
		// Get bone name
		bone_names[i].bone_name = bufferpos;

		// Go to bone offset
		bufferpos += bone_names[i].bone_name.size() + 1;

		// Copy bone offset
		memcpy((void *)&bone_names[i].bone_offset, bufferpos, sizeof(glm::mat4));

		// Go to next bone
		bufferpos += sizeof(float) * 16;
	}

	input.close();
}