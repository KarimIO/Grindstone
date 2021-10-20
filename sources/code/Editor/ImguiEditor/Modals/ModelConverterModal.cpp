#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "Editor/Importers/ModelImporter.hpp"
#include "ModelConverterModal.hpp"

const ImVec2 IMG_CONVERTER_WINDOW_SIZE = { 300.f, 100.f };

std::string GetModelAssetPath(std::string& path) {
	size_t p = path.find_last_of('/');
	return "../assets/" + path.substr(p + 1);
}

std::string GetGmfPath(std::string& path) {
	size_t p = path.find_last_of('.');
	return path.substr(0, p + 1) + "gmf";
}

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			ModelConverterModal::ModelConverterModal() {}

			void ModelConverterModal::Show() {
				isShown = true;
				modelPath = "";
			}

			void ModelConverterModal::ConvertFile() {
				isProcessing = true;
				std::string slashSwappedStr = modelPath;
				std::replace(slashSwappedStr.begin(), slashSwappedStr.end(), '\\', '/');
				std::string outputPath = GetGmfPath(GetModelAssetPath(slashSwappedStr));
				//ConvertTexture(slashSwappedStr, false, outputPath);
				isProcessing = false;
				Close();
			}

			void ModelConverterModal::Close() {
				isShown = false;
				ImGui::CloseCurrentPopup();
			}

			void ModelConverterModal::Render() {
				if (isShown) {
					ImGui::OpenPopup("Convert Model");
				}

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

				ImGui::SetNextWindowSize(IMG_CONVERTER_WINDOW_SIZE);
				if (ImGui::BeginPopupModal("Convert Model", false, flags)) {

					if (isProcessing) {
						ImGui::Text("Processing...");
					}
					else {
						ImGui::InputFloat("Model Scale", &modelScale);
						ImGui::InputText("Model Path", &modelPath);
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
						ImGui::Text(error.c_str());
						ImGui::PopStyleColor();

						if (ImGui::Button("Convert")) {
							ConvertFile();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							Close();
						}
					}
					
					ImGui::EndPopup();
				}
			}
		}
	}
}
