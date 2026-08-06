Plugin Registry {#PluginRegistry}
============

\warning These features are not yet implemented yet.

While currently, plugins must be available locally, we also want plugins to be available online. We hope to have a database of plugins
that a user can press a single button to install, hosted on a variety of sources. Each registry (with one official %Grindstone registry),
would make a list to various versions of various plugins. Initially, this will be a **JSON file in a Git repository**, containing:

- Plugin names
- Versions
- Sources (e.g.: git URLs with tags or commit hashes)

Users can add custom registries as well for theirself, their plugins, or for third party registries.

This design allows for the following features:

- Automatic updates
- Discoverability of new plugins
- Pluggable resolver support
