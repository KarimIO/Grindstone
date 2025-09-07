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

	if (document.HasMember("requiresRestart")) {
		rapidjson::Value& requiresRestartJson = document["requiresRestart"];
		if (requiresRestartJson.GetType() == rapidjson::Type::kTrueType || requiresRestartJson.GetType() == rapidjson::Type::kFalseType) {
			metaData.isRestartRequired = requiresRestartJson.GetBool();
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'requiresRestart' which must be of type 'false' or 'true'.\n", std::make_format_args(pathCstr));
		}
	}

	if (document.HasMember("assetDirectories")) {
		rapidjson::Value& assetDirectoriesJson = document["assetDirectories"];
		if (assetDirectoriesJson.GetType() == rapidjson::Type::kArrayType) {
			for (rapidjson::Value& directoryJson : assetDirectoriesJson.GetArray()) {
				if (directoryJson.GetType() == rapidjson::Type::kObjectType) {
					MetaData::AssetDirectory assetDirectory;

					if (directoryJson.HasMember("path")) {
						rapidjson::Value& pathJson = directoryJson["path"];
						if (pathJson.IsString()) {
							assetDirectory.assetDirectoryRelativePath = ParseBinaryPath(pathJson.GetString());
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has \'path\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has missing \'path\'.\n", std::make_format_args(pathCstr));
					}

					if (directoryJson.HasMember("mountPoint")) {
						rapidjson::Value& mountPointJson = directoryJson["mountPoint"];
						if (mountPointJson.IsString()) {
							assetDirectory.mountPoint = mountPointJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has \'mountPoint\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has missing \'mountPoint\'.\n", std::make_format_args(pathCstr));
					}

					if (directoryJson.HasMember("loadStage")) {
						rapidjson::Value& loadStageJson = directoryJson["loadStage"];
						if (loadStageJson.IsString()) {
							assetDirectory.loadStage = loadStageJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has \'loadStage\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'assetDirectories' element has missing \'loadStage\'.\n", std::make_format_args(pathCstr));
					}

					metaData.assetDirectories.emplace_back(assetDirectory);
				}
				else {
					errorMsg += std::vformat("Meta file {} has 'assetDirectories' has an element which should be of type object.\n", std::make_format_args(pathCstr));
				}
			}
		}
		else {
			errorMsg += std::vformat("Meta file {} has 'assetDirectories' which should be of type array.\n", std::make_format_args(pathCstr));
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

					if (binaryJson.HasMember("loadStage")) {
						rapidjson::Value& loadStageJson = binaryJson["loadStage"];
						if (loadStageJson.IsString()) {
							binary.loadStage = loadStageJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'binaries' element has \'loadStage\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}
					else {
						errorMsg += std::vformat("Meta file {} has 'binaries' element has missing \'loadStage\'.\n", std::make_format_args(pathCstr));
					}

					if (binaryJson.HasMember("dotnetTarget")) {
						rapidjson::Value& buildTargetJson = binaryJson["dotnetTarget"];
						if (buildTargetJson.IsString()) {
							binary.buildType = MetaData::BinaryBuildType::Dotnet;
							std::filesystem::path dotnetPath = metaDataFilePath.parent_path().filename() / buildTargetJson.GetString();
							binary.buildTarget = dotnetPath.string();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'binaries' element has \'dotnetTarget\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
					}

					if (binaryJson.HasMember("cmakeTarget")) {
						rapidjson::Value& buildTargetJson = binaryJson["cmakeTarget"];
						if (buildTargetJson.IsString()) {
							binary.buildType = MetaData::BinaryBuildType::Cmake;
							binary.buildTarget = buildTargetJson.GetString();
						}
						else {
							errorMsg += std::vformat("Meta file {} has 'binaries' element has \'cmakeTarget\' which should be of type string.\n", std::make_format_args(pathCstr));
						}
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
