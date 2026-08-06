Creating A Plugin {#CreatingAPlugin}
============

Creating plugins for %Grindstone is an integral part of developing using the engine. All functionality and assets must be stored in a plugin.

---

## Plugin Folder Structure

Each plugin lives in its own folder and contains:

- A `README.md` file that contains a detailed description of the plugin, how it works, what it registers, what assets it has, and more.
- `plugin.meta.json` (metadata)
- Compiled DLLs (e.g.: `bin/PluginRuntime.dll`, `bin/PluginEditor.dll`)
- Asset folders (e.g.: `assets/`)
- C++ Source files (`src/`, `include/`)
- C# Code files (`scripts/`)
- CMake file (`src/CMakeLists.txt`)

This structure allows:

- Multiple DLLs per plugin (e.g.: runtime/editor separation)
- Declarative loading order via dependency graph.
- Mounted assets.
- Automatic CMake building as part of the base solution.

---

### Plugin Metadata

Each plugin contains a `plugin.meta.json` manifest file, written in JSON. Here is an example from the Renderer plugin:
```json
{
	"name": "Grindstone.Renderer.Deferred",
	"displayName": "Deferred Renderer",
	"version": "0.3.0",
	"description": "The deferred renderer is a way of drawing the scene that focuses on drawing many lights simultaneously.",
	"author": "Grindstone Foundation",
	"requiresRestart": true,
	"assetDirectories": [
		{
			"path": "assets",
			"mountPoint": "GRINDSTONE.RENDERER.DEFERRED",
			"loadStage": "EditorAssetImportEarly"
		}
	],
	"dependencies": [],
	"binaries": [
		{
			"path": "lib/{Configuration}/PluginRendererDeferred",
			"loadStage": "EndOfEngineSetup",
			"cmakeTarget": "PluginRendererDeferred"
		}
	],
	"cmake": "CMakeLists.txt"
}
```

#### Base File format

| Field name        | Type                        | Description                                                                                                 |
|-------------------|-----------------------------|-------------------------------------------------------------------------------------------------------------|
| `name`            | string                      | A unique name for the plugin. It should be scoped, usually by Author.Type.PluginName.                       |
| `displayName`     | string                      | A human-readable name that makes sense to be shown in logs and editors.                                     |
| `version`         | string                      | A semvar - which shows the version in the format `Major`.`Minor`.`Patch`                                    |
| `description`     | string                      | A description to be shown in the plugin settings page.                                                      |
| `author`          | string                      | The name of the author (either a person, or perhaps a group.)                                               |
| `requiresRestart` | boolean                     | Signifies whether a plugin can gracefully be enabled/disabled or requires a restart to work.                |
| `assetDirectories`| Array of `AssetDirectory`   | A list of all the directories which can provide assets for the game or editor to use.                       |
| `dependencies`    | Array of `string`           | A list of dependencies for the plugin to require. If these plugins are not in the list, they will be loaded anyway. |
| `binaries`        | Array of `Binary`           | A list of code binaries to load - these will contain functionality to be registered into the game/editor.   |
| `cmake`           | string                      | The relative path to a CMakeLists.txt file which is used to build C++ code.                                 |

#### Binary format

| Field name        | Type                        | Description                                                                                                   |
|-------------------|-----------------------------|---------------------------------------------------------------------------------------------------------------|
| `path`            | string                      | The path to the actual compiled binary. Should be in lib if C++, scripts/bin if C#.                           |
| `loadStage`       | string                      | The stage in the loading of the game when a plugin can be loaded or unloaded. These are provided by the engine and editor.    |
| `cmakeTarget`     | string                      | The name of the target in the Cmake file. Multiple targets can be provided in `CmakeLists.txt`                |
| `dotnetTarget`    | string                      | The path to a json file which defines how the C# project can be generated. Should be in the `scripts` folder. |

Paths can use the special token `{Configuration}` which expands to `Debug`, `MinSizeRel`, `RelWithDebugInfo` or `Release`. The meaning of these are not too important to plugin developers but should be included regardless.

#### AssetDirectory format

| Field name        | Type                        | Description                                                                                                   |
|-------------------|-----------------------------|---------------------------------------------------------------------------------------------------------------|
| `path`            | string                      | The path to the actual compiled binary. Should be in lib if C++, scripts/bin if C#.                           |
| `loadStage`       | string                      | The stage in the loading of the game when a plugin can be loaded or unloaded. These are provided by the engine and editor.    |
| `mountPoint`      | string                      | This is a string that can be used as a root directory. It will be shown in the editor and can be used to refer to assets by path. |
#### List of Plugin Stages

As described above, asset directories and binaries can be loaded in the following broad stages:

 - `EndOfEngineSetup`
 - `EarlyEngineSetup`
 - `EditorAssetImportEarly`
 - `EditorAssetImportLate`
 - `EditorBeforeCameraInitialization`
 - `EditorAfterCameraInitialization`
 - `EditorBeforeSceneInitialization`
 - `EditorAfterSceneInitialization`
 - `EditorAfterUiSetup`

---

## C# binary file format

A plugin can declare one or more C# files. These can contain the binary `name`, and a list of `reference`s to other binary names.

\warning This will load the binaries in order. Please ensure no cyclic dependencies exist (i.e.: where plugin `A` requires plugin `B`, plugin `B` requires plugin `C`, and plugin `C` requires plugin `A` again.)

```json
{
	"name": "Grindstone.Ai.NavMesh.CSharpIntegration",
	"references": [
		"Grindstone.Script.CSharpCore"
	]
}
```
