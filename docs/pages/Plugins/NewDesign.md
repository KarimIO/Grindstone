# Plugin System 2.0 Specification

Although the [current plugin](Overview.md) system supports a lot of functionality, the upcoming system is more comprehensive.

---

## Plugin Manifests

Plugins appear in arbitrary order in a `.json` file and list:

- Semantic version constraints
- A source

Sources are resolved using **Plugin Source Resolvers**, which are pluggable. Examples include:

- `local`
- `git`
- `perforce`

Resolvers themselves can be plugins.

There are multiple plugin manifests used in a typical setup:

| Scope           | File                           | Managed By          | In Source Control?   |
|-----------------|--------------------------------|---------------------|----------------------|
| Project-wide    | `plugins.json`                 | Project             | ✅ Yes              |
| User per-project| `plugins.user.{USERNAME}.json` | Local developer     | 💭 Optional         |
| Global user     | `plugins.global.json`          | Local developer     | ❌ No               |

A `plugin.lock.json` lockfile is generated to capture resolved sources, versions, and commits. This lockfile **should be checked into source control** to ensure reproducibility.

---

## Plugin Registry

A central plugin registry is planned. Initially, this will be a **JSON file in a Git repository**, containing:

- Plugin names
- Versions
- Sources (e.g., git URLs with tags or commit hashes)

Users can add custom registries as well.

Benefits:

- Automatic updates
- Discoverability of new plugins
- Pluggable resolver support

---

## Plugin Folder Structure

Each plugin lives in its own folder and contains:

- `plugin.meta.json` (metadata)
- Compiled DLLs (e.g., `bin/PluginRuntime.dll`, `bin/PluginEditor.dll`)
- Asset folders (e.g., `assets/`)
- Source files (`src/`, `include/`)
- CMake file (`src/CMakeLists.txt`)

This structure allows:

- Multiple DLLs per plugin (e.g., runtime/editor separation)
- Declarative loading order via dependency graph
- Mounted assets
- Automatic CMake inclusion in the base solution

---

### Plugin Metadata

Each plugin contains a `plugin.meta.json` manifest file, written in JSON, that includes:

- Plugin name and version
- Metadata (description, author, etc.)
- Dependency list
- List of DLLs (editor/runtime/both)
- List of asset folders
- Source files and CMake file (optional)
- Load order priority
- Scope tags (e.g., `editor`, `runtime`, or `both`)
