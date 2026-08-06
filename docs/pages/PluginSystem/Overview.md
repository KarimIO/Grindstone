Plugin System {#PluginSystem}
============

The Plugin system is core to the design of %Grindstone. It allows games to be modular and extensible, without managing the overhead of unnecessary systems.

Plugins can be stored within a project, or downloaded from a network. When loaded, they should register themselves to expose their functionality to other plugins and the game code. When the plugin is unloaded, they must also clean up after themselves.

How to create a plugin:
 - @subpage CreatingAPlugin
 - @subpage RegisteringPluginCode

How the plugin ecosystem works:
 - @subpage RegisteringPlugins
 - @subpage PluginRegistry
