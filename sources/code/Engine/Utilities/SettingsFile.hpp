#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "GraphicsLanguage.hpp"
#include "Core/iniHandler.hpp"
#include <string>

const std::string settings_path = "../settings.ini";

class Settings {
public:
	Settings();
	bool loadSettings();
	void saveSettings();

	std::string default_map_;
	int resolution_x_;
	int resolution_y_;
	float fov_;
	bool vsync_;
	bool enable_ssao_;
	bool enable_reflections_;
	bool enable_tesselation_;
	bool enable_shadows_;
	bool show_pipeline_load_;
	bool show_material_load_;
	bool show_texture_load_;
	bool start_editor_;
	float mouse_sensitivity_;
	GraphicsLanguage graphics_language_;
private:
	bool loadSettings(INIConfigFile &cfile);
	void saveSettings(INIConfigFile &cfile);
};

#endif