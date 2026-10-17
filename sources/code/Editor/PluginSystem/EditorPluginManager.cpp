#include <Common/Graphics/Core.hpp>
#include <Common/Logger.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ScriptBuilder/CSharpProjectBuilder.hpp>
#include <Editor/ScriptBuilder/SolutionBuilder.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Grindstone.Script.CSharp/include/CSharpManager.hpp>

#include "EditorPluginManager.hpp"
#include "PluginManifestFileLoader.hpp"
#include "PluginManifestLockFileLoader.hpp"
#include "PluginMetaFileLoader.hpp"

#include <queue>
#include <ranges>
#include <rapidjson/document.h>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>

#include "PluginManifestData.hpp"
#include <variant>

using namespace Grindstone::Plugins;
using namespace Grindstone::Utilities;

constexpr const char* configuration = CMAKE_INTDIR;

static void ReadPipeLoop(HANDLE pipe, Grindstone::LogSeverity severity) {
	DWORD bytesRead;
	char buffer[4096]{ 0 };
	std::string partial;
	while (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
		partial.append(buffer, bytesRead);
		size_t pos;
		while ((pos = partial.find('\n')) != std::string::npos) {
			std::string line = partial.substr(0, pos);
			GPRINT(severity, Grindstone::LogSource::Editor, line.c_str());
			partial.erase(0, pos + 1);
		}
	}
	if (!partial.empty()) {
		GPRINT(severity, Grindstone::LogSource::Editor, partial.c_str()); // final line without newline
	}
}

static bool RunCommand(char* commandLine, const char* workingDirectory) {
	SECURITY_ATTRIBUTES securityAttribs{
		.nLength = sizeof(securityAttribs),
		.lpSecurityDescriptor = NULL,
		.bInheritHandle = TRUE
	};

	HANDLE stdoutRead, stdoutWrite, stderrRead, stderrWrite;
	CreatePipe(&stdoutRead, &stdoutWrite, &securityAttribs, 0);
	SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&stderrRead, &stderrWrite, &securityAttribs, 0);
	SetHandleInformation(stderrRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA si{
		.cb = sizeof(si),
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput = NULL,
		.hStdOutput = stdoutWrite,
		.hStdError = stderrWrite,
	};

	PROCESS_INFORMATION pi{};
	BOOL success = CreateProcessA(
		NULL,
		static_cast<LPSTR>(commandLine),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		workingDirectory,
		&si,
		&pi
	);

	CloseHandle(stdoutWrite);
	CloseHandle(stderrWrite);

	if (!success) {
		CloseHandle(stdoutRead);
		CloseHandle(stderrRead);
		return false;
	}

	std::thread readInfoThread(ReadPipeLoop, stdoutRead, Grindstone::LogSeverity::Info);
	std::thread readErrThread(ReadPipeLoop, stderrRead, Grindstone::LogSeverity::Error);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	readInfoThread.join();
	readErrThread.join();

	return true;
}

static bool RunCMakeCommand(const std::vector<std::string>& cmakeTargets) {
	std::string workingDirectory = EngineCore::GetInstance().GetEngineBinaryPath().parent_path().string();

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --build .";
	cmdStream << " --config " << configuration;
	cmdStream << " --parallel";
	cmdStream << " --target";
	for (const std::string& target : cmakeTargets) {
		cmdStream << " \"" << target << "\"";
	}
	std::string commandLine = cmdStream.str();
	return RunCommand(commandLine.data(), workingDirectory.empty() ? nullptr : workingDirectory.c_str());
}

static bool RunDotnetCommand(const std::filesystem::path& path) {
	std::string workingDirectory = (EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins").string();

	std::ostringstream cmdStream;
	cmdStream << "dotnet build ";
	cmdStream << "--configuration " << configuration << " ";
	cmdStream << '\"' << path.string() << '\"';
	std::string commandLine = cmdStream.str();
	return RunCommand(commandLine.data(), workingDirectory.empty() ? nullptr : workingDirectory.c_str());
}

static void CompileBinaries(const std::filesystem::path& solutionPath, const std::vector<Grindstone::Plugins::MetaData>& resolvedPluginManifest) {
	std::vector<std::string> cmakeTargetsToCompile;

	for (const Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		for (const Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (!binary.buildTarget.empty()) {
				switch (binary.buildType) {
				case MetaData::BinaryBuildType::NoBuild: {
					std::string path = binary.libraryRelativePath.string();
					GPRINT_ERROR(LogSource::Editor, "Binary {} has a target {} but no build type.", path.c_str(), binary.buildTarget.c_str());
					break;
				}
				case MetaData::BinaryBuildType::Cmake:
					cmakeTargetsToCompile.emplace_back(binary.buildTarget);
					break;
				case MetaData::BinaryBuildType::Dotnet:
					break;
				}
			}
			else if (binary.buildType != MetaData::BinaryBuildType::NoBuild) {
				std::string path = binary.libraryRelativePath.string();
				GPRINT_ERROR(LogSource::Editor, "Binary {} has a build type other than 'NoBuild' but not a target.", path.c_str());
			}
		}
	}

	return;

	if (!cmakeTargetsToCompile.empty()) {
		RunCMakeCommand(cmakeTargetsToCompile);
	}

	RunDotnetCommand(solutionPath);
}

struct AssemblyReference {
	std::string referenceName;
};

struct AssemblyDefinition {
	std::string name;
	std::filesystem::path projectPath;
	std::vector<AssemblyReference> references;
};

static std::optional<AssemblyDefinition> ParseAssemblyDefinition(std::filesystem::path path) {
	std::string pathStr = path.string();
	const char* pathCstr = pathStr.c_str();
	if (!std::filesystem::exists(path)) {
		GPRINT_ERROR(LogSource::Editor, "Assembly definition file doesn't exist {}", pathCstr);
		return std::nullopt;
	}

	std::string metaFileContents = Grindstone::Utils::LoadFileText(pathCstr);

	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(metaFileContents.c_str());

	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Failed to parse Assembly Definition file '{}'.", pathCstr);
		return std::nullopt;
	}

	AssemblyDefinition assemblyDefinition;

	if (document.HasMember("name")) {
		rapidjson::Value& nameJson = document["name"];
		if (nameJson.GetType() == rapidjson::Type::kStringType) {
			assemblyDefinition.name = nameJson.GetString();
		}
		else {
			GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Assembly definition file {} has 'name' which should be of type string.\n", pathCstr);
			return std::nullopt;
		}
	}
	else {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Assembly definition file {} has no required parameter 'name'.\n", pathCstr);
		return std::nullopt;
	}

	std::string warningMsg = "";
	const char* nameCString = assemblyDefinition.name.c_str();

	if (document.HasMember("references")) {
		rapidjson::Value& referencesJson = document["references"];
		if (referencesJson.GetType() == rapidjson::Type::kArrayType) {
			const auto& referencesArray = referencesJson.GetArray();
			for (const rapidjson::Value& referenceJson : referencesArray) {
				if (referenceJson.GetType() == rapidjson::Type::kStringType) {
					const char* reference = referenceJson.GetString();
					assemblyDefinition.references.emplace_back(
						AssemblyReference{
							.referenceName = reference
						}
					);
				}
				else {
					warningMsg += std::vformat("Assembly definition file {} has element in 'references' which should be of type string.\n", std::make_format_args(nameCString));
				}
			}
		}
		else {
			GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Assembly definition file {} has 'references' which should be of type array of strings.\n", nameCString);
			return std::nullopt;
		}
	}

	if (!warningMsg.empty()) {
		GPRINT_WARN(Grindstone::LogSource::EngineCore, "Assembly definition file {} was parsed successfully, but has warnings:\n{}", nameCString, warningMsg.c_str());
	}

	return assemblyDefinition;
}

static void MakeProjectsAndSolutions(const std::filesystem::path& solutionPath, const std::vector<Grindstone::Plugins::MetaData>& resolvedPluginManifest) {
	std::unordered_map<std::string, AssemblyDefinition> assembliesMap;

	for (const Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		for (const Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (!binary.buildTarget.empty() && binary.buildType == MetaData::BinaryBuildType::Dotnet) {
				std::filesystem::path filepath = metaData.pluginResolvedPath / binary.buildTarget;
				std::optional<AssemblyDefinition> assemblyDefinitionOpt = ParseAssemblyDefinition(filepath);

				if (assemblyDefinitionOpt.has_value()) {
					AssemblyDefinition& assemblyDefinition = assemblyDefinitionOpt.value();
					auto assemblyIt = assembliesMap.find(assemblyDefinition.name);
					if (assemblyIt != assembliesMap.end()) {
						GPRINT_WARN(LogSource::Editor, "Duplicate project called '{}' found!", assemblyDefinition.name.c_str());
					}
					else {
						assemblyDefinition.projectPath = filepath.parent_path() / (assemblyDefinition.name + ".csproj");
						assembliesMap[assemblyDefinition.name] = assemblyDefinition;
					}
				}
			}
		}
	}

	std::vector<std::filesystem::path> projectPaths;
	for (auto& assemblyIt : assembliesMap) {
		AssemblyDefinition& assemblyDefinition = assemblyIt.second;
		const std::filesystem::path& projectPath = assemblyDefinition.projectPath;
		projectPaths.emplace_back(projectPath);

		std::vector<std::filesystem::path> references;
		for (const AssemblyReference& ref : assemblyDefinition.references) {
			auto referenceAssembly = assembliesMap.find(ref.referenceName);
			if (referenceAssembly != assembliesMap.end()) {
				std::filesystem::path referencePath = referenceAssembly->second.projectPath;
				references.emplace_back(referencePath);
			}
			else {
				GPRINT_WARN(LogSource::Editor, "Unknown reference '{}' found in assembly '{}'!", ref.referenceName, assemblyDefinition.name.c_str());
			}
		}

		Grindstone::Editor::ScriptBuilder::CreateProjectFile(projectPath, references);
	}

	Grindstone::Editor::ScriptBuilder::CreateSolutionFile(solutionPath, projectPaths);
}

const std::vector<std::filesystem::path>& EditorPluginManager::GetPluginsFolders() {
	return pluginsFolders;
}

void EditorPluginManager::AddPluginsFolder(const std::filesystem::path& path) {
	GPRINT_INFO(LogSource::Editor, "Adding plugin search folder: {}", path.string());
	pluginsFolders.emplace_back(path);
}

void EditorPluginManager::ResolvePlugins(std::vector<Grindstone::Plugins::ManifestData>& manifestResults) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path();

	// TODO: Get dependencies that weren't in the lock file by passing to a queue in a while loop and checking each plugin's deps.
	for (Grindstone::Plugins::ManifestData& manifestData : manifestResults) {
		Grindstone::Plugins::MetaData metaData{};
		std::filesystem::path metaFilePath = basePath / manifestData.path / "plugin.meta.json";
		if (Grindstone::Plugins::ReadMetaFile(metaFilePath, metaData)) {
			resolvedPluginManifest.emplace_back(metaData);
		}
	}
}

EditorPluginManager::~EditorPluginManager() {
	for (auto it = pluginModules.rbegin(); it != pluginModules.rend(); ++it) {
		auto handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
			}
			else {
				std::string pluginName = it->first.string();
				GPRINT_ERROR(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {}", pluginName.c_str());
			}
		}
	}
}

bool EditorPluginManager::PreprocessPlugins() {
	std::vector<Grindstone::Plugins::ManifestData> manifestResults{};
	if (!Grindstone::Plugins::LoadPluginManifestFile(manifestResults)) {
		return false;
	}

	if (!Grindstone::Plugins::LoadPluginManifestLockFile(this, manifestResults)) {
		return false;
	}

	std::filesystem::path solutionPath = Editor::Manager::GetInstance().GetProjectPath();
	solutionPath = solutionPath / (solutionPath.filename().string() + ".slnx");

	std::vector<std::string_view> stageOrder{
		"EngineSystemsSetup",
		"EarlyEngineSetup",
		"JustBeforeEndOfEngineSetup",
		"EndOfEngineSetup",
		"EditorAfterEarlyInitialize",
		"EditorEarly",
		"EditorAssetImportEarly",
		"EditorAssetImportLate",
		"EditorAssetRegistryInitialization",
		"EditorBeforeSceneInitialization",
		"EditorAfterSceneInitialization",
		"EditorAfterUiSetup"
	};
	std::vector<std::string> outErrors;

	ResolvePlugins(manifestResults);
	if (!BuildLoadOrder(stageOrder)) {
		return false;
	}
	MakeProjectsAndSolutions(solutionPath, resolvedPluginManifest);
	CompileBinaries(solutionPath, resolvedPluginManifest);

	return true;
}


static std::vector<int> TopoSort(std::vector<std::vector<int>>& adj) {
	int n = adj.size();
	std::vector<int> indegree(n, 0);
	std::queue<int> q;
	std::vector<int> list;

	// Compute indegrees
	for (int i = 0; i < n; i++) {
		for (int next : adj[i]) indegree[next]++;
	}

	// Add all nodes with indegree 0 into the queue
	for (int i = 0; i < n; i++)
		if (indegree[i] == 0) q.push(i);

	// Kahn’s Algorithm (BFS)
	while (!q.empty()) {
		int top = q.front();
		q.pop();
		list.push_back(top);
		for (int next : adj[top]) {
			indegree[next]--;
			if (indegree[next] == 0) q.push(next);
		}
	}

	return list;
}

// Plugin-level reachability via requiresPlugins. Cycles are fine here,
// since this graph only decides inclusion, so a visited set is enough.
static bool IsPluginReachable(
	const std::unordered_map<std::string, const MetaData*>& pluginsByName,
	const std::string& from,
	const std::string& to
) {
	std::unordered_set<std::string> visited;
	std::vector<std::string> stack{from};
	while (!stack.empty()) {
		std::string current = std::move(stack.back());
		stack.pop_back();
		if (current == to) {
			return true;
		}

		if (!visited.insert(current).second) {
			continue;
		}

		auto it = pluginsByName.find(current);
		if (it == pluginsByName.end()) {
			continue;
		}

		for (const auto& req : it->second->requiresPlugins) {
			stack.push_back(req.pluginName);
		}
	}
	return false;
};

struct Node {
	LoadItem item;
	std::string qualifiedId;  // "Plugin:item"
	size_t stageIndex = 0;
	const std::vector<std::string>* dependencies = nullptr;
	std::vector<size_t> dependents;
	size_t pendingDependencies = 0;
};

static bool SortStage(
	const std::string_view stageName,
	size_t stage,
	const std::vector<std::vector<size_t>>& nodesByStage,
	std::vector<Node>& nodes,
	std::vector<LoadItem>& outSorted
) {
	std::priority_queue<size_t, std::vector<size_t>, std::greater<size_t>> ready;
	for (size_t i : nodesByStage[stage]) {
		if (nodes[i].pendingDependencies == 0) {
			ready.push(i);
		}
	}

	outSorted.clear();
	outSorted.reserve(nodesByStage[stage].size());

	while (!ready.empty()) {
		size_t current = ready.top();
		ready.pop();
		outSorted.push_back(nodes[current].item);

		for (size_t dependent : nodes[current].dependents) {
			if (--nodes[dependent].pendingDependencies == 0) {
				ready.push(dependent);
			}
		}
	}

	if (outSorted.size() != nodesByStage[stage].size()) {
		std::string message = std::format("Dependency cycle in stage '{}' involving: ", stageName);
		for (size_t i : nodesByStage[stage]) {
			if (nodes[i].pendingDependencies > 0) {
				message += " " + nodes[i].qualifiedId;
			}
		}
		GPRINT_ERROR(LogSource::Editor, "{}", std::move(message));
		return false;
	}

	return true;
}

bool EditorPluginManager::BuildLoadOrder(
	std::span<const std::string_view> stageOrder
) {
	using MetaData = Grindstone::Plugins::MetaData;

	orderedItemsByStage.clear();

	std::unordered_map<std::string_view, size_t> stageIndices;
	for (size_t i = 0; i < stageOrder.size(); ++i) {
		stageIndices.emplace(stageOrder[i], i);
	}

	std::unordered_map<std::string, const MetaData*> pluginsByName;
	for (const MetaData& plugin : resolvedPluginManifest) {
		pluginsByName.emplace(plugin.name, &plugin);
	}

	std::vector<Node> nodes;
	std::unordered_map<std::string, size_t> nodeIndexById;
	bool hasError = false;

	auto AddNode = [&stageIndices, &nodeIndexById, &nodes](
					   LoadItem item,
					   const std::string& itemName, const std::string& stage,
					   const std::vector<std::string>& dependencies) -> bool {
		std::string id = item.plugin->name + ":" + itemName;

		auto stageIt = stageIndices.find(stage);
		if (stageIt == stageIndices.end()) {
			GPRINT_ERROR(LogSource::Editor, "{}: unknown loadStage '{}'.", id, stage);
			return false;
		}

		if (!nodeIndexById.emplace(id, nodes.size()).second) {
			GPRINT_ERROR(LogSource::Editor, "{}: duplicate item name within plugin");
			return false;
		}

		Node node;
		node.item = item;
		node.qualifiedId = std::move(id);
		node.stageIndex = stageIt->second;
		node.dependencies = &dependencies;
		nodes.push_back(std::move(node));

		return true;
	};

	// Pass 1: Create nodes.
	for (const MetaData& plugin : resolvedPluginManifest) {
		for (const auto& binary : plugin.binaries) {
			LoadItem item{&plugin, &binary};
			if (!AddNode(item, binary.name, binary.loadStage, binary.dependencies)) {
				hasError = true;
			}
		}
		for (const auto& dir : plugin.assetDirectories) {
			LoadItem item{&plugin, &dir};
			if (!AddNode(item, dir.name, dir.loadStage, dir.dependencies)) {
				hasError = true;
			}
		}
	}

	// Pass 2: Resolve & Validate edges within and between stages.
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
		Node& node = nodes[nodeIndex];
		const std::string& ownerPlugin = node.item.plugin->name;

		for (const std::string& dependency : *node.dependencies) {
			// Bare name = same plugin, "Plugin:item" = cross-plugin.
			std::string depId = dependency.find(':') == std::string::npos
									? ownerPlugin + ":" + dependency
									: dependency;

			auto depIt = nodeIndexById.find(depId);
			if (depIt == nodeIndexById.end()) {
				GPRINT_ERROR(LogSource::PluginSystem, "{}: unknown dependency '{}'.", node.qualifiedId, dependency);
				hasError = true;
				continue;
			}

			size_t depIndex = depIt->second;
			const Node& depNode = nodes[depIndex];
			const std::string& depPlugin = depNode.item.plugin->name;

			if (depPlugin != ownerPlugin &&
				!IsPluginReachable(pluginsByName, ownerPlugin, depPlugin)) {
				GPRINT_ERROR(LogSource::PluginSystem, "{}: depends on '{}' but plugin '{}' does not require plugin '{}'.",
							 node.qualifiedId, depId, ownerPlugin, depPlugin);
				hasError = true;
				continue;
			}

			if (depNode.stageIndex > node.stageIndex) {
				GPRINT_ERROR(LogSource::PluginSystem, "{}: depends on '{}' which loads in a later stage",
							 node.qualifiedId, depId);
				hasError = true;
			}
			else if (depNode.stageIndex == node.stageIndex) {
				nodes[depIndex].dependents.push_back(nodeIndex);
				node.pendingDependencies++;
			}
			// If we have a dependency on an earlier stage, we don't need to worry about it as the stages are loaded independently..
		}
	}

	if (hasError) {
		return false;
	}

	// Pass 3: Sort stages independently.
	std::vector<std::vector<size_t>> nodesByStage(stageOrder.size());
	for (size_t i = 0; i < nodes.size(); ++i) {
		nodesByStage[nodes[i].stageIndex].push_back(i);
	}

	for (size_t stage = 0; stage < stageOrder.size(); ++stage) {
		const std::string_view stageName = stageOrder[stage];
		if (SortStage(stageName, stage, nodesByStage, nodes, orderedItemsByStage[std::string(stageName)])) {
			hasError = true;
		}
	}

	return !hasError;
}

void EditorPluginManager::LoadPluginsByStage(std::string_view stageName) {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();

	auto it = orderedItemsByStage.find(std::string(stageName));
	if (it == orderedItemsByStage.end()) {
		return;
	}

	for (const LoadItem& item : it->second) {
		if (std::holds_alternative<const MetaData::Binary*>(item.loadable)) {
			const MetaData::Binary& binary = *std::get<const MetaData::Binary*>(item.loadable);

			switch (binary.buildType) {
			case Grindstone::Plugins::MetaData::BinaryBuildType::Cmake: {
				std::filesystem::path binaryPath = item.plugin->pluginResolvedPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();
				DLL_DIRECTORY_COOKIE dllCookie = AddDllDirectory(parentPath.wstring().c_str());
				GPRINT_INFO(LogSource::Editor, "[PluginSystem] In stage {}, mounting C++ (cmake) plugin: {}", stageName, binary.libraryRelativePath.string());
				LoadModule(binaryPath);
				RemoveDllDirectory(dllCookie);
				break;
			}
			case Grindstone::Plugins::MetaData::BinaryBuildType::Dotnet: {
				auto scriptManager = Grindstone::EngineCore::GetInstance().TryGetService<Grindstone::Scripting::CSharp::CSharpManager>();
				GPRINT_INFO(LogSource::Editor, "[PluginSystem] In stage {}, mounting C# (dotnet) plugin: {}", stageName, binary.libraryRelativePath.string());
				scriptManager->LoadAssemblyIntoMap(item.plugin->name + ":" + binary.name);
				break;
			}
			}
		}
		else if (std::holds_alternative<const MetaData::AssetDirectory*>(item.loadable)) {
			const MetaData::AssetDirectory& assetDir = *std::get<const MetaData::AssetDirectory*>(item.loadable);
			std::filesystem::path assetsPath = item.plugin->pluginResolvedPath / assetDir.assetDirectoryRelativePath;
			Editor::FileManager& fileManager = editorManager.GetFileManager();
			GPRINT_INFO(LogSource::Editor, "[PluginSystem] In stage {}, mounted plugin asset directory {} to path: {}", stageName, assetDir.mountPoint, assetsPath.string());
			fileManager.MountDirectory(assetDir.mountPoint, assetsPath);
		}
		else {
			GS_ASSERT(false);
		}
	}
}

void EditorPluginManager::UnloadPluginsByStage(std::string_view stageName) {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();

	auto it = orderedItemsByStage.find(std::string(stageName));
	if (it == orderedItemsByStage.end()) {
		return;
	}

	for (const LoadItem& item : it->second | std::ranges::views::reverse) {
		const std::filesystem::path& pluginPath = item.plugin->pluginResolvedPath;
		if (std::holds_alternative<const MetaData::Binary*>(item.loadable)) {
			const MetaData::Binary& binary = *std::get<const MetaData::Binary*>(item.loadable);
			std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
			UnloadModule(binaryPath);
		}
		else if (std::holds_alternative<const MetaData::AssetDirectory*>(item.loadable)) {
			const MetaData::AssetDirectory& assetDir = *std::get<const MetaData::AssetDirectory*>(item.loadable);
			std::filesystem::path assetsPath = pluginPath / assetDir.assetDirectoryRelativePath;
			Editor::FileManager& fileManager = editorManager.GetFileManager();
			fileManager.UnmountDirectory(assetDir.mountPoint);
		}
	}
}

std::filesystem::path Grindstone::Plugins::EditorPluginManager::GetLibraryPath(std::string_view pluginName, std::string_view libraryName) {
	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		if (metaData.name == pluginName) {
			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				if (binary.name == libraryName) {
					std::filesystem::path filename = binary.libraryRelativePath.filename();
					filename = metaData.pluginResolvedPath / binary.libraryRelativePath.parent_path() / (filename.string() + ".dll");
					return filename;
				}
			}

			return "";
		}
	}

	return "";
}

bool EditorPluginManager::LoadModule(const std::filesystem::path& path) {
	auto it = pluginModules.find(path);
	if (it != pluginModules.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	auto handle = Modules::Load(path.string().c_str());

	if (handle) {
		pluginModules[path] = handle;

		auto initializeModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "InitializeModule");

		if (initializeModuleFnPtr) {
			initializeModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());

			return true;
		}
		else {
			GPRINT_ERROR(LogSource::EngineCore, "Unable to call InitializeModule in plugin: {0}", path.string().c_str());
			return false;
		}
	}

#ifdef _WIN32
	const DWORD lastError = GetLastError();
	char errorString[256] = {};
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorString,
		255,
		nullptr
	);

	GPRINT_ERROR(LogSource::EngineCore, "Unable to load plugin \"{0}\": {1}", path.string(), errorString);
#else
	GPRINT_ERROR(LogSource::EngineCore, "Unable to load plugin: {0}", path.string());
#endif

	return false;
}

void EditorPluginManager::UnloadModule(const std::filesystem::path& path) {
	auto it = pluginModules.find(path);
	if (it == pluginModules.end()) {
		GPRINT_ERROR(LogSource::EngineCore, "Unable to unload plugin \"{0}\"", path.string());
		return;
	}

	Grindstone::Utilities::Modules::Handle handle = it->second;
	if (handle) {
		auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

		if (releaseModuleFnPtr) {
			releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
		}
		else {
			GPRINT_ERROR(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", it->first.string().c_str());
		}

		Grindstone::Utilities::Modules::Unload(handle);
	}

	pluginModules.erase(it);
}

void EditorPluginManager::QueueInstall(const std::filesystem::path& pluginPath) {
	queuedInstalls.insert(pluginPath);

	if (queuedUninstalls.contains(pluginPath)) {
		queuedUninstalls.erase(pluginPath);
	}
}

void EditorPluginManager::QueueUninstall(const std::filesystem::path& pluginPath) {
	queuedUninstalls.insert(pluginPath);

	if (queuedInstalls.contains(pluginPath)) {
		queuedInstalls.erase(pluginPath);
	}
}

void EditorPluginManager::ProcessQueuedPluginInstallsAndUninstalls() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	if (!queuedUninstalls.empty()) {
		engineCore.GetGraphicsCore()->WaitUntilIdle();

		for (const std::filesystem::path& plugin : queuedUninstalls) {
			for (size_t metaDataIndex = 0; metaDataIndex < resolvedPluginManifest.size(); ++metaDataIndex) {
				Grindstone::Plugins::MetaData& metaData = resolvedPluginManifest[metaDataIndex];
				if (metaData.pluginResolvedPath == plugin) {
					std::filesystem::path pluginPath = metaData.pluginResolvedPath;
					for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
						std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
						UnloadModule(binaryPath);
					}

					resolvedPluginManifest.erase(resolvedPluginManifest.begin() + metaDataIndex);
					break;
				}
			}
		}
		queuedUninstalls.clear();
	}

	if (!queuedInstalls.empty()) {
		std::vector<ManifestData> pluginsToInstall;
		for (const std::filesystem::path& pluginPath : queuedInstalls) {
			pluginsToInstall.emplace_back(
				ManifestData{
					.pluginName = pluginPath.filename().string().c_str(),
					.semanticVersioning = ">0.0.1",
					.path = pluginPath
				}
			);
		}

		ResolvePlugins(pluginsToInstall);

		for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
			bool toInstall = false;
			for (const std::filesystem::path& pluginToInstall : queuedInstalls) {
				if (metaData.name == pluginToInstall) {
					toInstall = true;
				}
				break;
			}

			if (!toInstall) {
				continue;
			}

			std::filesystem::path pluginPath = metaData.pluginResolvedPath;

			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();
				DLL_DIRECTORY_COOKIE dllCookie = AddDllDirectory(parentPath.wstring().c_str());
				LoadModule(binaryPath);
				RemoveDllDirectory(dllCookie);
			}
		}

		queuedInstalls.clear();
	}
}

EditorPluginManager::Iterator EditorPluginManager::begin() noexcept {
	return resolvedPluginManifest.begin();
}

EditorPluginManager::ConstIterator EditorPluginManager::begin() const noexcept {
	return resolvedPluginManifest.begin();
}

EditorPluginManager::Iterator EditorPluginManager::end() noexcept {
	return resolvedPluginManifest.end();
}

EditorPluginManager::ConstIterator EditorPluginManager::end() const noexcept {
	return resolvedPluginManifest.end();
}
