Registering Plugins {#RegisteringPlugins}
============

After plugins are [created](@ref CreatingAPlugin), they will need to be registered so they can be used. Here are the current and future methods of how they can be used.

---

## Plugin List

The initial plugin system for %Grindstone is very simple. Projects contain a simple text file `buildplugins.txt` that lists each plugin separated by a newline. Each row is simply the name of a shared library (`.dll`, `.so`, etc.) without the extension. The `PluginManager` will load each file in the same order as that of the list.

An example of a plugin file is:
```plaintext
PluginEditorAudioImporter
PluginEditorMaterialImporter
PluginEditorModelImporter
PluginEditorPipelineSetImporter
PluginEditorTextureImporter
PluginBulletPhysics
PluginRenderables3D
PluginRendererDeferred
```

---

## Plugin Manifests (FUTURE WORK)

Plugin manifests are lists of **installed** plugins.

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
| User Global     | `plugins.global.json`          | Local developer     | ❌ No               |

A `plugin.lock.json` lockfile is generated to capture resolved sources, versions, and commits. This lockfile **should be checked into source control** to ensure reproducibility.
