#include "SettingsFile.hpp"
#include "Core/Utilities.hpp"
#include "Core/iniHandler.hpp"


Settings::Settings() {
	GRIND_PROFILE_FUNC();
	INIConfigFile cfile;

	if (!loadSettings(cfile)) {
		GRIND_WARN("SETTINGS.INI: File not found.");

		saveSettings(cfile);
	}
}

bool Settings::loadSettings(INIConfigFile &cfile) {
	cfile.SetPath(settings_path);
	if (cfile.ReadFile()) {
		cfile.GetBool("Window", "vsync", true, vsync_);
		cfile.GetInteger("Window", "resx", 1366, resolution_x_);
		cfile.GetInteger("Window", "resy", 768, resolution_y_);
		cfile.GetFloat("Window", "fov", 90, fov_);
		cfile.GetFloat("Input", "mouseSensitivity", 1.0, mouse_sensitivity_);
		std::string graphics;
		cfile.GetString("Renderer", "graphics", "OpenGL", graphics);
		cfile.GetBool("Renderer", "reflections", true, enable_reflections_);
		cfile.GetBool("Renderer", "tesselation", true, enable_tesselation_);
		cfile.GetBool("Renderer", "shadows", true, enable_shadows_);
		cfile.GetBool("Renderer", "useSSAO", false, enable_ssao_);
		cfile.GetBool("Debug", "showMaterialLoad", true, show_material_load_);
		cfile.GetBool("Debug", "showPipelineLoad", true, show_pipeline_load_);
		cfile.GetBool("Debug", "showTextureLoad", false, show_texture_load_);
		cfile.GetString("Game", "defaultmap", "../assets/scenes/sponza.json", default_map_);
		cfile.GetBool("Game", "startWithEditor", false, start_editor_);

		graphics = strToLower(graphics);
		if (graphics == "directx")
			graphics_language_ = GraphicsLanguage::DirectX;
		else if (graphics == "vulkan")
			graphics_language_ = GraphicsLanguage::Vulkan;
		else if (graphics == "metal")
			graphics_language_ = GraphicsLanguage::Metal;
		else if (graphics == "opengl")
			graphics_language_ = GraphicsLanguage::OpenGL;
		else {
			GRIND_ERROR("SETTINGS.INI: Invalid value for graphics language ({0}), using Opengl instead.", graphics.c_str());
			graphics_language_ = GraphicsLanguage::OpenGL;
			cfile.SetString("Renderer", "graphics", "OpenGL");
		}

		cfile.SaveFile();
		return true;
	}

	return false;
}

void Settings::saveSettings(INIConfigFile &cfile) {
	resolution_x_ = 1366;
	resolution_y_ = 768;
	graphics_language_ = GraphicsLanguage::OpenGL;
	fov_ = 90.0f;
	enable_reflections_ = true;
	enable_shadows_ = true;
	enable_tesselation_ = true;
	vsync_ = true;
	show_material_load_ = 0;
	show_pipeline_load_ = 0;
	show_texture_load_ = 0;
	enable_ssao_ = true;
	mouse_sensitivity_ = 0.005f;
	start_editor_ = false;
	default_map_ = "../assets/scenes/sponza.json";

	cfile.SetBool("Window", "vsync", vsync_);
	cfile.SetInteger("Window", "resx", resolution_x_);
	cfile.SetInteger("Window", "resy", resolution_y_);
	cfile.SetFloat("Window", "fov", fov_);
	cfile.SetFloat("Input", "mouseSensitivity", mouse_sensitivity_);
	cfile.SetString("Renderer", "graphics", "OpenGL");
	cfile.SetBool("Renderer", "reflections", enable_reflections_);
	cfile.SetBool("Renderer", "shadows", enable_shadows_);
	cfile.SetBool("Renderer", "tesselation", enable_tesselation_);
	cfile.SetBool("Renderer", "useSSAO", enable_ssao_);
	cfile.SetBool("Debug", "showMaterialLoad", show_material_load_);
	cfile.SetBool("Debug", "showPipelineLoad", show_pipeline_load_);
	cfile.SetBool("Debug", "showTextureLoad", show_texture_load_);
	cfile.SetString("Game", "defaultmap", default_map_);
	cfile.SetBool("Game", "startWithEditor", start_editor_);
	cfile.SaveFile();
}

bool Settings::loadSettings() {
	INIConfigFile cfile;
	cfile.SetPath(settings_path);
	return loadSettings(cfile);
}

void Settings::saveSettings() {
	INIConfigFile cfile;
	cfile.SetPath(settings_path);
	saveSettings(cfile);
}