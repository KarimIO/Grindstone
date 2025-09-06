# Vulkan Render Hardware Interface

This plugin is developed by the Grindstone Foundation. It's designed to allow the application to render computer graphics using the Vulkan API.

## Registered Items

### Vulkan (Graphics Core)

The VulkanCore allows the application to render graphics. Vulkan is the system Grindstone supports best because it is one of the newer systems, and works on many platforms.

### WindowingManager

Every RHI plugin introduces a windowing system. That's because windowing systems (and general platform systems) are heavily intertwined with graphics, but they shouldn't necessarily be so closely tied. This may be changed in the future. The windowing manager allows the game to create windows (such as the editor or game windows).

### DisplayManager

Every RHI plugin introduces a DisplayManager system. That's because windowing systems (and general platform systems) are heavily intertwined with graphics, but they shouldn't necessarily be so closely tied. This will likely be fixed in the future. The DisplayManager makes the application aware of various monitors and displays.
