#include <filesystem>
#include <fstream>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "MaterialInspector.hpp"
#include "BrowseFile.hpp"
using namespace rapidjson;

std::string readTextFileMat(const char* filename) {
	std::ifstream ifs(filename);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			MaterialInspector::MaterialInspector(EngineCore* engineCore) : engineCore(engineCore) {}
			void MaterialInspector::setMaterialPath(const char* materialPath) {
				this->materialPath = materialPath;
			}

			void MaterialInspector::render() {
				ImGui::Text("Editing Material: %s", materialPath.c_str());
				ImGui::InputText("Material Name", &materialName);
				if (BrowseFile(engineCore, "Shader Path", shaderPath)) {
					tryLoadShaderReflection();
				}

				if (!shaderPath.empty()) {
					ImGui::Separator();

					if (!hasLoadFile) {
						ImGui::Text("Cannot load shader file");
						return;
					}
					ImGui::Text("Using shader: %s", shaderName.c_str());
					renderTextures();
					renderParameters();
				}
			}

			void MaterialInspector::tryLoadShaderReflection() {
				if (!std::filesystem::exists(shaderPath)) {
					hasLoadFile = false;
					return;
				}

				std::string buffer = readTextFileMat("..\\test.reflect.json");
				rapidjson::Document document;
				document.Parse(buffer.c_str());
				shaderName = document["name"].GetString();
				hasLoadFile = true;

				loadShaderUniformBuffers(document);
			}

			void MaterialInspector::loadShaderUniformBuffers(rapidjson::Document& document) {
				if (!document.HasMember("uniformBuffers")) {
					return;
				}

				auto& uniformBuffers = document["uniformBuffers"];
				materialUniformBuffers.reserve(uniformBuffers.Size());
				for (
					rapidjson::Value* itr = uniformBuffers.Begin();
					itr != uniformBuffers.End();
					++itr
				) {
					auto& uniformBuffer = itr->GetObject();
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
						auto& memberData = memberItr->GetObject();
						auto name = memberData["name"].GetString();
						size_t offset = memberData["offset"].GetUint();
						size_t memberSize = memberData["memberSize"].GetUint();
						memberList.emplace_back(name, offset, memberSize);
					}
				}
			}

			void MaterialInspector::renderTextures() {
				if (textures.size() == 0) {
					return;
				}
				
				ImGui::Separator();
				ImGui::Text("Textures:");

				for each (auto texture in textures) {
					renderTexture(texture);
				}
			}

			void MaterialInspector::renderParameters() {
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
					renderParameter(parameter);
				}
			}

			void MaterialInspector::renderTexture(MaterialTexture& texture) {

			}

			void MaterialInspector::renderParameter(MaterialParameter& parameter) {
				for each (auto& uniformBuffer in materialUniformBuffers) {
					if (ImGui::TreeNode(uniformBuffer.name.c_str())) {
						for each (auto& member in uniformBuffer.members) {
							ImGui::Text(member.name.c_str());
						}
						ImGui::TreePop();
					}
				}
			}
		}
	}
}
