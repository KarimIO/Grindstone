#include <regex>
#include <rapidjson/document.h>

#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>

#include "PluginMetaFileLoader.hpp"

static std::string ParseBinaryPath(const char* path) {
	std::regex re("\\{Configuration\\}");
	return std::regex_replace(path, re, CMAKE_INTDIR);
}

bool Grindstone::Plugins::ReadMetaFile(std::filesystem::path metaDataFilePath, Grindstone::Plugins::MetaData& metaData) {
	if (!std::filesystem::exists(metaDataFilePath)) {
		GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Couldn't open Plugin MetaData file '{}'.", metaDataFilePath.string());
		return false;
	}

	std::string pathString = metaDataFilePath.string();
	const char* pathCstr = pathString.c_str();
	std::string metaFileContents = Grindstone::Utils::LoadFileText(pathCstr);

	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(metaFileContents.c_str());

	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}
		GPRINT_ERROR_V(Grindstone::LogSource::EngineCore, "Failed to parse MetaData file '{}'.", metaDataFilePath.string());
		return false;
	}

	std::string errorMsg = "";
	bool hasError = false;
	if (document.HasMember("name")) {
		rapidjson::Value& nameJson = document["name"];
		if (nameJson.GetType() == rapidjson::Type::kStringType) {
			metaData.name = nameJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'name' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}
	else {
		errorMsg += std::vformat("Meta file {} has no required parameter 'name'.\n", std::make_format_args(pathCstr));
	}

	// TODO: Undo this hack of using the path from the path - the name should match!
	std::string parentDirectoryName = metaDataFilePath.parent_path().filename().string();
	if (metaData.name != parentDirectoryName) {
		metaData.name = parentDirectoryName;
	}

	if (document.HasMember("displayName")) {
		rapidjson::Value& displayNameJson = document["displayName"];
		if (displayNameJson.GetType() == rapidjson::Type::kStringType) {
			metaData.displayName = displayNameJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'displayName' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("version")) {
		rapidjson::Value& versionJson = document["version"];
		if (versionJson.GetType() == rapidjson::Type::kStringType) {
			metaData.version = versionJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'version' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}
	else {
		errorMsg += std::vformat("Meta file {} has no required parameter 'version'.\n", std::make_format_args(pathCstr));
	}

	if (document.HasMember("pluginLoadStage")) {
		rapidjson::Value& pluginLoadStageJson = document["pluginLoadStage"];
		if (pluginLoadStageJson.GetType() == rapidjson::Type::kStringType) {
			metaData.loadStage = pluginLoadStageJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'pluginLoadStage' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}
	else {
		errorMsg += std::vformat("Meta file {} has no required parameter 'pluginLoadStage'.\n", std::make_format_args(pathCstr));
	}

	if (document.HasMember("description")) {
		rapidjson::Value& descriptionJson = document["description"];
		if (descriptionJson.GetType() == rapidjson::Type::kStringType) {
			metaData.description = descriptionJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'description' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("author")) {
		rapidjson::Value& authorJson = document["author"];
		if (authorJson.GetType() == rapidjson::Type::kStringType) {
			metaData.author = authorJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'author' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("assets")) {
		rapidjson::Value& assetsJson = document["assets"];
		if (assetsJson.GetType() == rapidjson::Type::kArrayType) {
			for (auto& assetJson : assetsJson.GetArray()) {
				if (assetJson.GetType() == rapidjson::Type::kStringType) {
					metaData.assets.emplace_back(assetJson.GetString());
				}
				else {
					errorMsg += std::vformat("Meta file {} has 'assets' has an element which should be of type string.\n", std::make_format_args(pathCstr));
				}
			}
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'assets' which should be of type array.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("dependencies")) {
		rapidjson::Value& dependenciesJson = document["dependencies"];
		if (dependenciesJson.GetType() == rapidjson::Type::kArrayType) {
			for (rapidjson::Value& dependencyJson : dependenciesJson.GetArray()) {
				if (dependencyJson.GetType() == rapidjson::Type::kObjectType) {
					MetaData::Dependency dependency;

					if (dependencyJson.HasMember("dependencyName")) {
						rapidjson::Value& dependencyNameJson = dependencyJson["dependencyName"];
						if (dependencyNameJson.IsString()) {
							dependency.pluginName = dependencyNameJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'dependencies' element has \'dependencyName\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'dependencies' element has missing \'dependencyName\'.\n", std::make_format_args(pathCstr));
					}

					if (dependencyJson.HasMember("version")) {
						rapidjson::Value& versionJson = dependencyJson["version"];
						if (versionJson.IsString()) {
							dependency.version = versionJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'dependencies' element has \'version\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'dependencies' element has missing \'version\'.\n", std::make_format_args(pathCstr));
					}

					metaData.dependencies.emplace_back(dependency);
				}
				else {
					errorMsg += std::vformat("Meta file {} has 'dependencies' has an element which should be of type object.\n", std::make_format_args(pathCstr));
				}
			}
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'dependencies' which should be of type array.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("binaries")) {
		rapidjson::Value& binariesJson = document["binaries"];
		if (binariesJson.GetType() == rapidjson::Type::kArrayType) {
			for (rapidjson::Value& binaryJson : binariesJson.GetArray()) {
				if (binaryJson.GetType() == rapidjson::Type::kObjectType) {
					MetaData::Binary binary;

					if (binaryJson.HasMember("path")) {
						rapidjson::Value& pathJson = binaryJson["path"];
						if (pathJson.IsString()) {
							binary.libraryRelativePath = ParseBinaryPath(pathJson.GetString());
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'binaries' element has \'path\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'binaries' element has missing \'path\'.\n", std::make_format_args(pathCstr));
					}

					metaData.binaries.emplace_back(binary);
				}
				else {
					errorMsg += std::vformat("Meta file {} has 'binaries' has an element which should be of type object.\n", std::make_format_args(pathCstr));
				}
			}
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'binaries' which should be of type array.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("cmake")) {
		rapidjson::Value& cmakeJson = document["cmake"];
		if (cmakeJson.GetType() == rapidjson::Type::kStringType) {
			metaData.cmakePath = cmakeJson.GetString();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'cmake' which should be of type string.\n", std::make_format_args(pathCstr));
		}
	}

	if (errorMsg.size() > 1) {
		errorMsg[errorMsg.size() - 1] = '\0';
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, errorMsg.c_str());
		return false;
	}

	return true;
}
