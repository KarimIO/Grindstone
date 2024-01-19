#include <imgui.h>
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "StatsPanel.hpp"

#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Textures/TextureImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "Plugins/Renderables3D/Assets/Mesh3dImporter.hpp"

namespace Grindstone::Editor::ImguiEditor {
	StatsPanel::StatsPanel() {
		lastRenderTime = std::chrono::steady_clock::now();
	}

	void StatsPanel::Render() {
		if (isShowingPanel) {
			ImGui::Begin("Stats", &isShowingPanel);
			RenderContents();
			ImGui::End();
		}
	}

	void StatsPanel::RenderContents() {
		totalFrameCount++;
		frameCountSinceLastRender++;

		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		long long elapsedTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRenderTime).count();
		if (elapsedTimeMicroSeconds > 300000) {
			lastRenderTime = currentTime;
			lastRenderedTotalFrameCount = totalFrameCount;
			lastRenderedDeltaTime = elapsedTimeMicroSeconds / (1000000.0 * frameCountSinceLastRender);
			frameCountSinceLastRender = 0;
		}

		ImGui::Text("Total Frame Count: %I64u", totalFrameCount);
		ImGui::Text("Frame Time (Seconds): %f", lastRenderedDeltaTime);
		ImGui::Text("Frames Per Second: %f", 1 / lastRenderedDeltaTime);

		EngineCore& engineCore = Editor::Manager::GetEngineCore();

		if (ImGui::TreeNode("Materials")) {
			MaterialImporter* materialImporter = engineCore.assetManager->GetManager<MaterialImporter>();
			if (materialImporter == nullptr) {
				ImGui::Text("Material Importer unavailable");
			}
			else {
				for (auto& material : *materialImporter) {
					MaterialAsset& materialAsset = material.second;
					ImGui::Text("%s (%lu)", materialAsset.name.c_str(), materialAsset.referenceCount);
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Textures")) {
			TextureImporter* textureImporter = engineCore.assetManager->GetManager<TextureImporter>();
			if (textureImporter == nullptr) {
				ImGui::Text("Texture Importer unavailable");
			}
			else {
				for (auto& texture : *textureImporter) {
					TextureAsset& textureAsset = texture.second;
					ImGui::Text("%s (%lu)", textureAsset.name.c_str(), textureAsset.referenceCount);
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Shaders")) {
			ShaderImporter* shaderImporter = engineCore.assetManager->GetManager<ShaderImporter>();
			if (shaderImporter == nullptr) {
				ImGui::Text("Shader Importer unavailable");
			}
			else {
				for (auto& shader : *shaderImporter) {
					ShaderAsset& shaderAsset = shader.second;
					ImGui::Text("%s (%lu)", shaderAsset.name.c_str(), shaderAsset.referenceCount);
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Meshes")) {
			Mesh3dImporter* meshImporter = engineCore.assetManager->GetManager<Mesh3dImporter>();
			if (meshImporter == nullptr) {
				ImGui::Text("Mesh Importer unavailable");
			}
			else {
				for (auto& mesh : *meshImporter) {
					Mesh3dAsset& meshAsset = mesh.second;
					ImGui::Text("%s (%lu)", meshAsset.name.c_str(), meshAsset.referenceCount);
				}
			}

			ImGui::TreePop();
		}
	}
}
