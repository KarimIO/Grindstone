#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "Editor/Converters/ImageConverter.hpp"
#include "ImageConverterModal.hpp"

const ImVec2 IMG_CONVERTER_WINDOW_SIZE = { 300.f, 100.f };

std::string GetAssetPath(std::string& path) {
	size_t p = path.find_last_of('/');
	return "../assets/" + path.substr(p + 1);
}

std::string GetDdsPath(std::string& path) {
	size_t p = path.find_last_of('.');
	return path.substr(0, p + 1) + "dds";
}

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			ImageConverterModal::ImageConverterModal() {}

			void ImageConverterModal::show() {
				isShown = true;
				imagePath = "";
			}

			void ImageConverterModal::convertFile() {
				isProcessing = true;
				std::string slashSwappedStr = imagePath;
				std::replace(slashSwappedStr.begin(), slashSwappedStr.end(), '\\', '/');
				std::string outputPath = GetDdsPath(GetAssetPath(slashSwappedStr));
				ConvertTexture(slashSwappedStr, false, outputPath);
				isProcessing = false;
				close();
			}

			void ImageConverterModal::close() {
				isShown = false;
				ImGui::CloseCurrentPopup();
			}

			void ImageConverterModal::render() {
				if (isShown) {
					ImGui::OpenPopup("Convert Image");
				}

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

				ImGui::SetNextWindowSize(IMG_CONVERTER_WINDOW_SIZE);
				if (ImGui::BeginPopupModal("Convert Image", false, flags)) {

					if (isProcessing) {
						ImGui::Text("Processing...");
					}
					else {
						ImGui::InputText("Image Path", &imagePath);
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
						ImGui::Text(error.c_str());
						ImGui::PopStyleColor();

						if (ImGui::Button("Convert")) {
							convertFile();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							close();
						}
					}
					
					ImGui::EndPopup();
				}
			}
		}
	}
}
