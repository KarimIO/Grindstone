# Deferred Renderer

This plugin is developed by the Grindstone Foundation. It is a high level system built on top of the Render Hardware Interface.

## Registered Items

### DeferredRendererFactory (Renderer Factory)

The DeferredRendererFactory creates renderers which the Camera Component uses. Deferred Renderers can support many more lights than Forward Renderers but at the expense of a more complex geometry rendering path.

## Future Work

The main goals of the DeferredRenderer in the near future is to move towards GPU-driven geometry drawing (to improve performance with complex geometry), implement a cluster renderer (to improve performance with many lights), and to use a rendergraph / framegraph (to improve maintainability and extensibility).
