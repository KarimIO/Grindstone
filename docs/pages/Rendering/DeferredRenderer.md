# DeferredRenderer  

The **DeferredRenderer** is the default rendering technique used in the Grindstone Engine. It is designed for flexibility and efficiency, particularly in scenes with many dynamic lights.

---

## What is Deferred Rendering?

In traditional forward rendering, each object is rendered and lit in a single pass. This means each light has to be considered for every object, which becomes inefficient with many lights.

**Deferred rendering** separates rendering into two main stages:

1. **Geometry Pass** – Renders scene geometry into a set of G-buffers (geometry buffers)
2. **Lighting Pass** – Applies lighting using the data from the G-buffers

This approach means geometry is only drawn once, and lights are processed independently, making it ideal for scenes with high light counts.

---

## Summary of Render Stages

1. **Opaque Geometry Pass**  
   - Writes Diffuse, Normal, SpecularRoughness, and Depth attachments.
   - Uses `GeometryOpaque`

2. **Screen Space Ambient Occlusion Pass**  
   - Detects for geometry that is close to each other from the opaque pass, in order to increase the shadow around contact areas.

3. **Lighting Pass**  
   - Per-light fullscreen passes of point lights, spot lights, and directional lights.
   - Image-based lighting pass that uses light probes to add ambient detail and reflections (using spherical harmonics for ambient and cubemaps for reflections).

4. **Unlit Geometry Pass**  
   - Uses `GeometryUnlit`  
   - Unlit geometry on top.

5. **Sky Geometry Pass**  
   - Uses `GeometrySky`  
   - Forward-rendered skies, which appear at the back of the scene.

6. **Transparent Geometry Pass**  
   - Uses `GeometryTransparent`  
   - Forward-rendered transparent objects, which are blended on top (glass, particles, etc).

7. **Post-Processing**  
   - Enhances the look of the scene after geometry and lighting.

### Opaque Geometry Pass

The renderer writes material and surface data to several textures, known as attachments or G-buffers:

- **Diffuse** – stores surface diffuse color
- **SpecularRoughness** – stores surface specular color and roughness
- **Normals** – stores surface normal directions in view space
- **Depth** – stores pixel depth (also used to reconstruct world positions)

These are written using calls to `assetRendererManager->RenderQueue(commandBuffer, registry, "GeometryOpaque")`, which asks each registered `AssetRenderer` (such as `Mesh3dRenderer`) to render all opaque objects.

### Lighting Pass

Once geometry is captured, lighting is applied:

- **Per-light passes**: Each light (point, directional, spot) is rendered as a **full-screen quad**, using shaders that read from the G-buffers to compute lighting at each pixel.
- **Image-Based Lighting (IBL)**: The renderer applies global lighting using an environment cubemap (for specular reflections) and spherical harmonics (for diffuse lighting). This simulates ambient lighting from the surrounding world.

### Unlit, Sky, and Transparent Passes

After lighting, unlit, sky, and transparent transparent objects are rendered separately using:
```cpp
assetRendererManager->RenderQueue(commandBuffer, registry, "GeometryUnlit");
assetRendererManager->RenderQueue(commandBuffer, registry, "GeometrySky");
assetRendererManager->RenderQueue(commandBuffer, registry, "GeometryTransparent");
```

---

### Post-Processing

Following the lighting stage, the `DeferredRenderer` runs a post-processing pipeline, which includes:

- **Screen Space Ambient Occlusion (SSAO)** - This calculates how much the ambient lighting will be able to reach certain areas, where less lighting can reach corners and areas with high detail.
- **Screen Space Reflections (SSR) (Work In Progress)** - Reflect rays on the geometry as it has been already rendered, to be able to render reflections. This only works if the reflection results in a position that is on the screen.
- **Depth of Field (DOF) (Work In Progress)** - Breaks the geometry into slices - near, far, and mid-distance, where near and far places may be blurred differently.
- **Bloom** - Takes extremely bright pixels and blurs them.
- **Composite** - This does many operations in one go:
    - *Chromatic Abberation* - shifts colors slightly by channel (red, green, and blue channels).
    - *Compositing Bloom* - takes the result of the Bloom process and combines it with the regular image.
    - *Vignette* - darkens the edges of the image.
    - *Grain* - applies static noise to the image.
    - *Tonemapping* - takes a high dynamic range color (0-Infinity colors) and tones it down to standard dynamic range (0-255 colors).
    - *Linear to SRGB* - applies gamma correction to the image.

These effects refine the final image for realism and mood.

### Final Output

The final image, as with any Renderer, is redirected to one of two places - the swapchain (monitor), or another render target to be used elsewhere (like the editor viewport).

## Future Work

- **Screen Space Reflections** for higher fidelity reflections
- **Depth of Field** for blurring very close or very far geometry, like a real lens does
- **Improved light culling** using meshes (spheres for point lights, and cones for spot lights), and tiled or clustered light culling. This reduces the pixels which are lit for each light.