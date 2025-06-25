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
#include "Editor/ImguiEditor/ImguiEditor.hpp"

#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <Plugins/GraphicsVulkan/VulkanDescriptorSet.hpp>

using namespace Grindstone::Editor::ImguiEditor;

MaterialInspector::MaterialInspector(EngineCore* engineCore, ImguiEditor* imguiEditor) : engineCore(engineCore), imguiEditor(imguiEditor) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();

	Grindstone::GraphicsAPI::DescriptorSetLayout::Binding binding{
		0, // descriptor index
		1, // count
		Grindstone::GraphicsAPI::BindingType::SampledImage,
		Grindstone::GraphicsAPI::ShaderStageBit::Fragment
	};

	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCreateInfo;
	layoutCreateInfo.debugName = "Material Inspector Descriptor Layout";
	layoutCreateInfo.bindings = &binding;
	layoutCreateInfo.bindingCount = 1;
	textureDisplayDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(layoutCreateInfo);

	std::array<unsigned char, 16> colorData = {
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
		0x00, 0x00, 0x00, 0xff,
	};

	GraphicsAPI::Image::CreateInfo blackTextureCreateInfo{};
	blackTextureCreateInfo.debugName = "MaterialInspector Missing Image";
	blackTextureCreateInfo.initialData = reinterpret_cast<const char*>(&colorData);
	blackTextureCreateInfo.initialDataSize = static_cast<uint32_t>(colorData.size());
	blackTextureCreateInfo.width = 2;
	blackTextureCreateInfo.height = 2;
	blackTextureCreateInfo.format = GraphicsAPI::Format::R8G8B8A8_UNORM;
	blackTextureCreateInfo.mipLevels = 1;
	blackTextureCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::TransferSrc |
		GraphicsAPI::ImageUsageFlags::TransferDst |
		GraphicsAPI::ImageUsageFlags::Sampled;
	missingImage = graphicsCore->CreateImage(blackTextureCreateInfo);

	GraphicsAPI::DescriptorSet::Binding dsetBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage(missingImage);
	GraphicsAPI::DescriptorSet::CreateInfo missingImageDescriptorSetCreateInfo{};
	missingImageDescriptorSetCreateInfo.debugName = "MaterialInspector Missing Image Descriptor Set";
	missingImageDescriptorSetCreateInfo.bindings = &dsetBinding;
	missingImageDescriptorSetCreateInfo.bindingCount = 1;
	missingImageDescriptorSetCreateInfo.layout = textureDisplayDescriptorSetLayout;
	missingImageDescriptorSet = graphicsCore->CreateDescriptorSet(missingImageDescriptorSetCreateInfo);
}

void MaterialInspector::SetMaterialPath(const std::filesystem::path& materialPath) {
	if (this->materialPath == materialPath) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	ReloadAvailablePipelineSets();

	for (auto& sampler : pipelineSetSamplers) {
		graphicsCore->DeleteDescriptorSet(sampler.textureDescriptorSet);
	}

	pipelineSetSamplers.clear();
	parameters.clear();
	hasBeenChanged = false;
	this->materialPath = materialPath;
	filename = materialPath.filename().string();
	std::string materialPathAsString = materialPath.string();
	std::string contentData = Grindstone::Utils::LoadFileText(materialPathAsString.c_str());
	shaderLoadStatus = ShaderLoadStatus::Unassigned;
	pipelineSetName = "";
	shaderUuid = Grindstone::Uuid();

	rapidjson::Document document;
	if (document.Parse(contentData.data()).GetParseError()) {
		GPRINT_ERROR(LogSource::Editor, "Invalid Material.");
		materialName = filename;
		shaderUuid = Uuid();
		return;
	}

	if (document.HasMember("name")) {
		materialName = document["name"].GetString();
	}
	else {
		GPRINT_ERROR(LogSource::Editor, "No name found in material.");
	}

	if (!document.HasMember("shader")) {
		GPRINT_ERROR(LogSource::Editor, "No shader found in material.");
		return;
	}

	if (!Grindstone::Uuid::MakeFromString(document["shader"].GetString(), shaderUuid)) {
		return;
	}

	TryLoadShaderReflection(shaderUuid);

	if (document.HasMember("samplers")) {
		LoadMaterialSamplers(document["samplers"]);
	}
}

void MaterialInspector::ReloadAvailablePipelineSets() {
	availablePipelineSets.clear();

	AssetRegistry& registry = Editor::Manager::GetInstance().GetAssetRegistry();
	registry.FindAllFilesOfType(AssetType::GraphicsPipelineSet, availablePipelineSets);

	std::sort(
		availablePipelineSets.begin(), availablePipelineSets.end(),
		[](AssetRegistry::Entry& a, AssetRegistry::Entry& b) {
			return a.displayName < b.displayName;
		}
	);
}

void MaterialInspector::Render() {
	ImGui::Text("Editing Material: %s", filename.c_str());
	if (ImGui::InputText("Material Name", &materialName)) {
		hasBeenChanged = true;
	}

	if (ImGui::BeginCombo("Graphics Pipeline Set", pipelineSetName.c_str())) {
		for (size_t i = 0; i < availablePipelineSets.size(); ++i) {
			bool isCurrentShader = availablePipelineSets[i].uuid == shaderUuid;
			if (ImGui::Selectable(availablePipelineSets[i].displayName.c_str(), isCurrentShader)) {
				pipelineSetName = availablePipelineSets[i].displayName;
				shaderUuid = availablePipelineSets[i].uuid;

				TryLoadShaderReflection(shaderUuid);

				hasBeenChanged = true;
			}
		}
		ImGui::EndCombo();
	}

	if (shaderLoadStatus != ShaderLoadStatus::ValidShader) {
		ImGui::Text("Shader not loaded");
		return;
	}

	RenderTextures();
	RenderParameters();

	ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 24 };

	if (hasBeenChanged) {
		if (ImGui::Button("Apply", buttonSize)) {
			SaveMaterial();
		}
	}
	else {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::Button("Apply", buttonSize);
		ImGui::PopStyleVar();
	}
}

bool MaterialInspector::TryLoadShaderReflection(Uuid& shaderUuid) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;

	Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> graphicsPipelineSet = assetManager->GetAssetReferenceByUuid<GraphicsPipelineAsset>(shaderUuid);
	if (!graphicsPipelineSet.IsValid()) {
		shaderLoadStatus = ShaderLoadStatus::NoFileFound;
		return false;
	}

	Grindstone::GraphicsPipelineAsset* gpset = graphicsPipelineSet.Get();
	if (gpset == nullptr) {
		shaderLoadStatus = ShaderLoadStatus::InvalidShader;
		return false;
	}

	shaderLoadStatus = ShaderLoadStatus::ValidShader;

	pipelineSetSamplers.clear();
	for (auto& resource : gpset->metaData.resources) {
		pipelineSetSamplers.emplace_back(resource.slotName.c_str());
	}

	return true;
}

void MaterialInspector::LoadShaderUniformBuffers(rapidjson::Value& uniformBuffers) {
	materialUniformBuffers.reserve(uniformBuffers.Size());
	for (
		rapidjson::Value* itr = uniformBuffers.Begin();
		itr != uniformBuffers.End();
		++itr
	) {
		rapidjson::Value& uniformBuffer = *itr;
		const char* name = uniformBuffer["name"].GetString();
		size_t bindingId = uniformBuffer["binding"].GetUint();
		size_t bufferSize = uniformBuffer["bufferSize"].GetUint();
		materialUniformBuffers.emplace_back(name, bindingId, bufferSize);
		rapidjson::Value& memberSource = uniformBuffer["members"];
		auto& memberList = materialUniformBuffers.back().members;
		memberList.reserve(memberSource.Size());
		for (
			rapidjson::Value* memberItr = memberSource.Begin();
			memberItr != memberSource.End();
			++memberItr
		) {
			rapidjson::Value& memberData = *memberItr;
			const char* name = memberData["name"].GetString();
			size_t offset = memberData["offset"].GetUint();
			size_t memberSize = memberData["memberSize"].GetUint();
			memberList.emplace_back(name, offset, memberSize);
		}
	}
}

void MaterialInspector::LoadMaterialSamplers(rapidjson::Value& materialSamplers) {
	Grindstone::Assets::AssetManager* assetManager = engineCore->assetManager;
	Grindstone::Editor::AssetRegistry* assetRegistry = &Editor::Manager::GetInstance().GetAssetRegistry();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();

	for (size_t index = 0; index < pipelineSetSamplers.size(); ++index) {
		auto& shaderSampler = pipelineSetSamplers[index];
		const char* samplerName = shaderSampler.name.c_str();
		if (materialSamplers.HasMember(samplerName)) {
			std::string samplerValue = materialSamplers[samplerName].GetString();
			Grindstone::Uuid uuid;
			AssetRegistry::Entry entry;

			if (Grindstone::Uuid::MakeFromString(samplerValue, uuid)) {
				if (assetRegistry->TryGetAssetData(uuid, entry)) {
					shaderSampler.value = samplerValue;
					shaderSampler.valueName = entry.displayName;
					hasBeenChanged = false;
					shaderSampler.textureReference = assetManager->GetAssetReferenceByUuid<Grindstone::TextureAsset>(uuid);
				}
			}
			else if (assetRegistry->TryGetAssetData(samplerValue, entry)) {
				shaderSampler.value = samplerValue;
				shaderSampler.valueName = entry.displayName;
				hasBeenChanged = false;
				shaderSampler.textureReference = assetManager->GetAssetReferenceByAddress<Grindstone::TextureAsset>(samplerValue);
			}

			if (shaderSampler.textureReference.IsValid() && shaderSampler.textureReference.Get() != nullptr) {
				Grindstone::GraphicsAPI::Image* texturePtr = shaderSampler.textureReference.Get()->image;

				Grindstone::GraphicsAPI::DescriptorSet::Binding binding =
					Grindstone::GraphicsAPI::DescriptorSet::Binding::SampledImage(texturePtr);

				Grindstone::GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCi;
				std::string debugName = "Material Importer Dset " + entry.displayName;
				descriptorSetCi.debugName = debugName.c_str();
				descriptorSetCi.layout = textureDisplayDescriptorSetLayout;
				descriptorSetCi.bindings = &binding;
				descriptorSetCi.bindingCount = 1;
				shaderSampler.textureDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCi);
			}
			else {
				shaderSampler.textureDescriptorSet = nullptr;
			}
		}
	}
}

void MaterialInspector::RenderTextures() {
	if (pipelineSetSamplers.size() == 0) {
		return;
	}
				
	ImGui::Separator();
	ImGui::Text("Textures:");

	if (ImGui::BeginTable("MaterialTextureSlots", 2)) {
		for (auto& sampler : pipelineSetSamplers) {
			RenderTexture(sampler);
		}
		ImGui::EndTable();
	}
}

void MaterialInspector::RenderParameters() {
	for (auto & uniformBuffer : materialUniformBuffers) {
		if (ImGui::TreeNode(uniformBuffer.name.c_str())) {
			for (auto & member : uniformBuffer.members) {
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

	for(auto& parameter : parameters) {
		RenderParameter(parameter);
	}
}

void MaterialInspector::OnSelectedTexture(Uuid uuid, std::string name) {
	selectedSampler->value = uuid;
	selectedSampler->valueName = name;
	hasBeenChanged = true;

	selectedSampler->textureReference = engineCore->assetManager->GetAssetReferenceByUuid<Grindstone::TextureAsset>(uuid);

	if (selectedSampler->textureReference.IsValid() && selectedSampler->textureReference.Get() != nullptr) {
		Grindstone::GraphicsAPI::Image* texturePtr = selectedSampler->textureReference.Get()->image;

		Grindstone::GraphicsAPI::DescriptorSet::Binding binding =
			Grindstone::GraphicsAPI::DescriptorSet::Binding::SampledImage(texturePtr);

		Grindstone::GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCi;
		std::string debugName = "Material Importer Dset " + name;
		descriptorSetCi.debugName = debugName.c_str();
		descriptorSetCi.layout = textureDisplayDescriptorSetLayout;
		descriptorSetCi.bindings = &binding;
		descriptorSetCi.bindingCount = 1;
		selectedSampler->textureDescriptorSet = engineCore->GetGraphicsCore()->CreateDescriptorSet(descriptorSetCi);
	}
	else {
		selectedSampler->textureDescriptorSet = nullptr;
	}
}

void MaterialInspector::RenderTexture(Sampler& sampler) {
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text(sampler.name.c_str());

	ImGui::TableNextColumn();

	Grindstone::GraphicsAPI::DescriptorSet* dset = sampler.textureDescriptorSet != nullptr
		? sampler.textureDescriptorSet
		: missingImageDescriptorSet;

	Grindstone::GraphicsAPI::Vulkan::DescriptorSet* vdset = static_cast<Grindstone::GraphicsAPI::Vulkan::DescriptorSet*>(dset);
	ImTextureID descriptor = (ImTextureID)(uint64_t)(vdset->GetDescriptorSet());

	std::string imageText = (sampler.valueName + "##Img" + sampler.name);
	ImGui::Image(descriptor, ImVec2(40.0f, 40.0f));

	std::string buttonText = (sampler.valueName + "##" + sampler.name);
	ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 24 };
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
	if (ImGui::Button(buttonText.c_str(), buttonSize)) {
		selectedSampler = &sampler;
		std::function<void(Uuid, std::string)> callback = [this](Uuid uuid, std::string path) { OnSelectedTexture(uuid, path); };
		imguiEditor->PromptAssetPicker(AssetType::Texture, callback);
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
			Uuid uuid = *static_cast<Uuid*>(payload->Data);
			AssetRegistry::Entry entry;
			if (Editor::Manager::GetInstance().GetAssetRegistry().TryGetAssetData(uuid, entry)) {
				selectedSampler = &sampler;
				OnSelectedTexture(uuid, entry.displayName);
			}
		}

		ImGui::EndDragDropTarget();
	}
	ImGui::PopStyleVar();
}

void MaterialInspector::RenderParameter(MaterialParameter& parameter) {
	for (auto& uniformBuffer : materialUniformBuffers) {
		if (ImGui::TreeNode(uniformBuffer.name.c_str())) {
			for (auto& member : uniformBuffer.members) {
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
		documentWriter.String(shaderUuid.ToString().c_str());
	}
	{
		documentWriter.Key("parameters");
		documentWriter.StartObject();
		documentWriter.EndObject();
	}
	{
		documentWriter.Key("samplers");
		documentWriter.StartObject();
		for (auto& sampler : pipelineSetSamplers) {
			documentWriter.Key(sampler.name.c_str());
			documentWriter.String(sampler.value.c_str());
		}
		documentWriter.EndObject();
	}
	documentWriter.EndObject();

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(materialPath);
	file.write(content, strlen(content));
	file.flush();
	file.close();
}
