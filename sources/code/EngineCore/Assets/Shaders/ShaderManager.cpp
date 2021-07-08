#include "ShaderManager.hpp"
using namespace Grindstone;

Shader& ShaderManager::LoadShader(const char* path) {
	Shader* shader = nullptr;
	if (TryGetShader(path, shader)) {
		return *shader;
	}

	return CreateShaderFromFile(path);
}

bool ShaderManager::TryGetShader(const char* path, Shader*& shader) {
	auto& sh = shaders.find(path);
	if (sh != shaders.end()) {
		shader = &sh->second;
		return true;
	}

	return false;
}

Shader& ShaderManager::CreateShaderFromFile(const char* path) {
	shaders[path] = Shader{ path };
	auto& shader = shaders[path];
	CreateShaderGraphicsPipeline(path, shader);
	CreateReflectionDataForShader(path, shader);

	return shader;
}

void ShaderManager::CreateReflectionDataForShader(const char* path, Shader& shader) {
}

void ShaderManager::CreateShaderGraphicsPipeline(const char* path, Shader& shader) {
}
