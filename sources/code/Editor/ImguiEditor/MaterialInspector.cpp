#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "MaterialInspector.hpp"
#include "BrowseFile.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/EditorManager.hpp"
using namespace rapidjson;

using namespace Grindstone::Editor::ImguiEditor;

MaterialInspector::MaterialInspector(EngineCore* engineCore) : engineCore(engineCore) {}
void MaterialInspector::SetMaterialPath(const std::filesystem::path& materialPath) {
	if (this->materialPath == materialPath) {
		return;
	}

	samplers.clear();
	parameters.clear();
	hasBeenChanged = false;
	this->materialPath = materialPath;
	filename = materialPath.filename().string();
	std::string materialPathAsString = materialPath.string();
	std::string contentData = Grindstone::Utils::LoadFileText(materialPathAsString.c_str());

	rapidjson::Document document;
	document.Parse(contentData.data());

	if (document.HasMember("name")) {
		materialName = document["name"].GetString();
	}
	else {
		Editor::Manager::GetInstance().Print(LogSeverity::Error, "No name found in material.");
	}

	if (!document.HasMember("shader")) {
		Editor::Manager::GetInstance().Print(LogSeverity::Error, "No shader found in material.");
		return;
	}

	shaderUuid = Uuid(document["shader"].GetString());
	TryLoadShaderReflection(shaderUuid);

	if (document.HasMember("samplers")) {
		LoadMaterialSamplers(document["samplers"]);
	}
}

void MaterialInspector::Render() {
	ImGui::Text("Editing Material: %s", filename.c_str());
	ImGui::InputText("Material Name", &materialName);

	if (shaderLoadStatus != ShaderLoadStatus::ValidShader) {
		ImGui::Text("Shader not loaded");
		return;
	}
	else {
		ImGui::Text("Using shader: %s", shaderName.c_str());
	}

	RenderTextures();
	RenderParameters();

	if (hasBeenChanged) {
		if (ImGui::Button("Apply")) {
			SaveMaterial();
		}
	}
	else {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::Button("Apply");
		ImGui::PopStyleVar();
	}
}

bool MaterialInspector::TryLoadShaderReflection(Uuid& shaderUuid) {
	std::filesystem::path shaderPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / shaderUuid.ToString();
	if (!std::filesystem::exists(shaderPath)) {
		shaderLoadStatus = ShaderLoadStatus::NoFileFound;
		return false;
	}

	std::string contentData = Grindstone::Utils::LoadFileText(shaderPath.string().c_str());
	rapidjson::Document document;
	document.Parse(contentData.c_str());
	if (document.HasMember("name")) {
		shaderName = document.HasMember("name")
			? document["name"].GetString()
			: "Untitled Shader";
	}

	shaderLoadStatus = ShaderLoadStatus::ValidShader;


	if (document.HasMember("uniformBuffers")) {
		// LoadShaderUniformBuffers(document["uniformBuffers"]);
	}

	if (document.HasMember("samplers")) {
		LoadShaderTextures(document["samplers"]);
	}

	return true;
}

void MaterialInspector::LoadShaderTextures(rapidjson::Value& texturesInShader) {
	materialUniformBuffers.reserve(texturesInShader.Size());
	for (
		rapidjson::Value* sampler = texturesInShader.Begin();
		sampler != texturesInShader.End();
		++sampler
	) {
		if (sampler->HasMember("name")) {
			auto samplerName = (*sampler)["name"].GetString();
			samplers.emplace_back(samplerName);
		}
	}
}

void MaterialInspector::LoadShaderUniformBuffers(rapidjson::Value& uniformBuffers) {
	materialUniformBuffers.reserve(uniformBuffers.Size());
	for (
		rapidjson::Value* itr = uniformBuffers.Begin();
		itr != uniformBuffers.End();
		++itr
	) {
		auto& uniformBuffer = *itr;
		auto name = uniformBuffer["name"].GetString();
		size_t bindingId = uniformBuffer["binding"].GetUint();
		size_t bufferSize = uniformBuffer["bufferSize"].GetUint();
		materialUniformBuffers.emplace_back(name, bindingId, bufferSize);
		auto& memberSource = uniformBuffer["members"];
		auto& memberList = materialUniformBuffers.back().members;
		memberList.reserve(memberSource.Size());
		for (
			rapidjson::Value* memberItr = memberSource.Begin();
			memberItr != memberSource.End();
			++memberItr
		) {
			auto& memberData = *memberItr;
			auto name = memberData["name"].GetString();
			size_t offset = memberData["offset"].GetUint();
			size_t memberSize = memberData["memberSize"].GetUint();
			memberList.emplace_back(name, offset, memberSize);
		}
	}
}

void MaterialInspector::LoadMaterialSamplers(rapidjson::Value& samplers) {
	for (auto& shaderSampler : this->samplers) {
		const char* samplerName = shaderSampler.name.c_str();
		if (samplers.HasMember(samplerName)) {
			const char* samplerValue = samplers[samplerName].GetString();
			shaderSampler.value = Uuid(samplerValue);
		}
	}
}

void MaterialInspector::RenderTextures() {
	if (samplers.size() == 0) {
		return;
	}
				
	ImGui::Separator();
	ImGui::Text("Textures:");

	for (auto& sampler : samplers) {
		RenderTexture(sampler);
	}
}

void MaterialInspector::RenderParameters() {
	for each (auto & uniformBuffer in materialUniformBuffers) {
		if (ImGui::TreeNode(uniformBuffer.name.c_str())) {
			for each (auto & member in uniformBuffer.members) {
				ImGui::Text(member.name.c_str());
			}
			ImGui::TreePop();
		}
	}

	if (parameters.size() == 0) {
		return;
	}

	ImGui::Separator();
	ImGui::Text("Parameters:");

	for each(auto parameter in parameters) {
		RenderParameter(parameter);
	}
}

void MaterialInspector::RenderTexture(Sampler& sampler) {
	std::string buttonText = (sampler.value.ToString() + "#" + sampler.name);
	ImGui::Button(buttonText.c_str());

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_UUID")) {
			sampler.value = *static_cast<Uuid*>(payload->Data);
			hasBeenChanged = true;
		}

		ImGui::EndDragDropTarget();
	}
}

void MaterialInspector::RenderParameter(MaterialParameter& parameter) {
	for each (auto& uniformBuffer in materialUniformBuffers) {
		if (ImGui::TreeNode(uniformBuffer.name.c_str())) {
			for each (auto& member in uniformBuffer.members) {
				ImGui::Text(member.name.c_str());
			}
			ImGui::TreePop();
		}
	}
}

void MaterialInspector::SaveMaterial() {
	hasBeenChanged = false;

	rapidjson::StringBuffer documentStringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);

	documentWriter.StartObject();
	{
		documentWriter.Key("name");
		documentWriter.String(materialName.c_str());
	}
	{
		documentWriter.Key("shader");
		documentWriter.String(materialName.c_str());
	}
	{
		documentWriter.Key("parameters");
		documentWriter.StartArray();
		documentWriter.EndArray();
	}
	{
		documentWriter.Key("samplers");
		documentWriter.StartArray();
		for (auto& sampler : samplers) {
			documentWriter.Key(sampler.name.c_str());
			documentWriter.String(sampler.value.ToString().c_str());
		}
		documentWriter.EndArray();
	}
	documentWriter.EndObject();

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(materialPath);
	file.write(content, strlen(content));
	file.flush();
	file.close();
}
