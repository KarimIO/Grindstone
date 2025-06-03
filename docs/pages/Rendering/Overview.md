# Grindstone Rendering Overview

The rendering system in Grindstone is built for flexibility and extensibility. It is structured around a core concept of **Renderers**, which transform camera and world data into render commands for the GPU. This document provides an overview of how rendering works within the Grindstone Engine.

---

## What Is a Renderer?

A **Renderer** in Grindstone is responsible for turning a camera view of the world into rendered output. The engine uses a renderer associated with each camera (e.g., for split-screen game views, editor view, etc).

Renderers are created using a **RendererFactory**, which stores objects and settings common to all renderer instances, like shared resources or shader configurations. The RenderFactory is set by a plugin.

Every frame, the engine renders the game using the following inputs:
- A camera
- The world and its components (through a registry)
- A RenderPass and Framebuffer.

The renderer then does the following:
 - Orchestrating visible Geometry rendering
 - Apply lighting
 - Apply post-processing effects
 - Output final result to the screen or render target

Grindstone will support two Renderer plugins out of the box:
 - The **[DeferredRenderer](DeferredRenderer.md)** uses a geometry buffer (G-buffer) to render all geometry's material data (depth, normals, diffuse color, specular color, and roughness) in one pass, and then perform lighting calculations in a separate pass.  
 - **Forward+ rendering** (planned) performs tiled light culling and applies lighting directly while drawing geometry.

---

## Asset Renderers and Render Queues

To support rendering various asset types, Grindstone uses **AssetRenderer**s (e.g., `Mesh3dRenderer`). These are specialized classes that know how to render specific asset types. We also intend to support `TerrainRenderer`, `ParticleRenderer`, and `SkeletalMeshRenderer`.

Asset renderers are invoked using:

```cpp
assetRendererManager->RenderQueue(commandBuffer, registry, "GeometryOpaque");
```

Render queues are used to organize the order of rendering:
- `GeometryDepthPrepass`: Draw all depth-tested opaque geometry to reduce the cost of drawing the pixels fully
- `GeometryOpaque`: For depth-tested opaque lit geometry
- `GeometrySky`: Draw skies — this is done separately because they appear behind anything else
- `GeometryUnlit`: For depth-tested opaque objects that are unlit
- `GeometryTransparent`: For blended objects
- Others may be introduced for shadows, UI, etc.

---

## Materials, Meshes, and GraphicsPipelineSets

- **[Materials](Materials.md)** define how surfaces are shaded and [textured](Textures.md). They link to a [GraphicsPipelineSets](PipelineSets.md), a high-level abstraction of shaders and GPU state.
- **[Meshes](Meshes.md)** are the geometric data used for rendering.
- The combination of a mesh, a material, and a pipeline set determines how something is drawn.

Materials are configurable and can be edited in the Grindstone Editor’s Inspector Panel. They can be attached to an object via a MeshRendererComponent, whereas Meshes can be attached to an object via a MeshComponent. They are both required to render static geometry.

---

## Post-Processing

Post-Processing steps refine and stylize the rendered output after lighting is complete. These are defined by the individual renderers. These steps include **Screen Space Ambient Occlusion**, **Screen Space Reflections**, **Depth of Field**, **Bloom**, **Chromatic Abberation**, **Vignette**, **Film Grain**, and **Tonemapping**.

---

## Render Hardware Interface (RHI)

At the lowest level, Grindstone relies on a **Render Hardware Interface (RHI)** to abstract over graphics APIs. The RHI is implemented as a set of base interfaces, with concrete implementations in plugins for:

- Vulkan *(PluginGraphicsVulkan)*
- OpenGL *(PluginGraphicsOpenGL)*

This makes the engine portable and extensible for different platforms and GPU backends. This includes objects such as `Graphics Core`, `Buffer`, `Command Buffer`, `Descriptor Set`, `Descriptor Set Layout`, `Framebuffer`, `Graphics Pipeline`, `Compute Pipeline`, `Image`, `RenderPass`, `Sampler`, `Vertex Array Object`, `Window Graphics Binding`.

---

## Future Work

- **Separate shadow mapping** so that shadow maps are not rendered for each camera separately, and are instead rendered for all cameras in one go and then used in the final renderer.
- **Terrain Rendering** to draw vast landscapes efficiently.
- **Particle Rendering** to simulate and draw many particles on the screen (2d sprites or 3d models).
- **Skeletal Mesh Renderer** to draw animated characters.
- **GPU-driven bindless rendering** for high-performance instancing and indirect draws
- **Framegraph system** for better control over resource usage and render pass scheduling
- **Forward+ Renderer** as a plug-in alternative to deferred rendering
- **Modular Post-Processing Pipeline** to configure effects as reusable components
