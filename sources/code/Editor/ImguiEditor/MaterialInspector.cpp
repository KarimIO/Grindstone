#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "MaterialInspector.hpp"
#include "BrowseFile.hpp"

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

				ImGui::Separator();

				renderTextures();
				renderParameters();
			}

			void MaterialInspector::tryLoadShaderReflection() {
				
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

			}
		}
	}
}
