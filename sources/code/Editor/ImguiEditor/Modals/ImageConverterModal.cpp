#include <imgui.h>
#include <imgui_stdlib.h>
#include "Editor/Importers/TextureImporter.hpp"
#include "ImageConverterModal.hpp"
using namespace Grindstone::Editor::ImguiEditor;

const ImVec2 IMG_CONVERTER_WINDOW_SIZE = { 400.f, 250.f };

std::string GetAssetPath(std::string& path) {
	size_t p = path.find_last_of('/');
	return "../assets/" + path.substr(p + 1);
}

std::string GetDdsPath(std::string& path) {
	size_t p = path.find_last_of('.');
	if (p > 1) {
		return path.substr(0, p + 1) + "dds";
	}

	return path + ".dds";
}

std::string getImageOutputPath(std::string path) {
	return GetDdsPath(GetAssetPath(path));
}

const char* IMAGE_FORMATS[] = {
	"Autodetect",
	"Uncompressed",
	"BC1 - RGB (With optional Cutoff)",
	"BC3 - RGBA",
	// "BC4 - Greyscale",
	// "BC5 - RG",
	// "BC6 - HDR Image",
	// "BC7 - High Quality RGBA"
};

ImageConverterModal::ImageConverterModal() {}

void ImageConverterModal::Show() {
	isShown = true;
	imagePath = "";
	shouldImportCubemap = false;
	selectedFormatIndex = 0;
}

void ImageConverterModal::ConvertFile() {
	isProcessing = true;
	std::filesystem::path inputPath = GetInputPathWithProperSlashes();
	try {
		Grindstone::Importers::ImportTexture(inputPath);
	}
	catch (const char* error) {
		this->error = error;
		isProcessing = false;
		return;
	}
	isProcessing = false;
	Close();
}

std::string ImageConverterModal::GetInputPathWithProperSlashes() {
	std::string slashSwappedStr = imagePath;
	std::replace(slashSwappedStr.begin(), slashSwappedStr.end(), '\\', '/');
	return slashSwappedStr;
}

void ImageConverterModal::Close() {
	isShown = false;
	ImGui::CloseCurrentPopup();
}

void ImageConverterModal::RenderFormatCombo() {

	if (ImGui::BeginCombo("Image Format", IMAGE_FORMATS[selectedFormatIndex])) {
		for (char i = 0; i < IM_ARRAYSIZE(IMAGE_FORMATS); i++) {
			bool isSelected = (i == selectedFormatIndex);
			if (ImGui::Selectable(IMAGE_FORMATS[i], isSelected)) {
				selectedFormatIndex = i;
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
}

void ImageConverterModal::Render() {
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
			RenderFormatCombo();
			ImGui::InputText("Image Path", &imagePath);
			ImGui::Checkbox("Import as cubemap", &shouldImportCubemap);
			if (shouldImportCubemap) {
				size_t p = imagePath.find_last_of('.');
				if (p == -1) {
					p = imagePath.size();
				}

				std::string pre = imagePath.substr(0, p);
				std::string ext = imagePath.substr(p);

				ImGui::Text("Loading as:");
				ImGui::Text(("\t" + pre + "_ft" + ext).c_str());
				ImGui::Text(("\t" + pre + "_bk" + ext).c_str());
				ImGui::Text(("\t" + pre + "_up" + ext).c_str());
				ImGui::Text(("\t" + pre + "_dn" + ext).c_str());
				ImGui::Text(("\t" + pre + "_rt" + ext).c_str());
				ImGui::Text(("\t" + pre + "_lf" + ext).c_str());

				if (imagePath != "") {
					ImGui::Text("Outputting as %s", getImageOutputPath(pre + "_ft" + ext).c_str());
				}
			}
			else {
				if (imagePath != "") {
					ImGui::Text("Outputting as %s", getImageOutputPath(imagePath).c_str());
				}
			}
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			ImGui::Text(error.c_str());
			ImGui::PopStyleColor();

			if (ImGui::Button("Cancel")) {
				Close();
			}
			if (imagePath != "") {
				ImGui::SameLine();
				if (ImGui::Button("Convert")) {
					ConvertFile();
				}
			}
		}
					
		ImGui::EndPopup();
	}
}
