
# Textures

Textures are image data used to add detail and realism to 3D models and surfaces in your game. In **Grindstone**, **TextureAssets** allow developers and artists to bring surfaces to life — whether it's the diffuse color of a wall, the detail of a normal map, or the panoramic view of a skybox.

---

## What Is a Texture Asset?

A **TextureAsset** in Grindstone is an imported image, usually used to drive material appearance. It’s referenced in material samplers or directly used in shaders to provide surface-level data. Textures influence not just color, but also lighting, surface detail, and more.

---

## Supported Image Formats

Grindstone supports importing the following image file formats:

- `.bmp`
- `.jpg` / `.jpeg`
- `.tga`
- `.png`
- `.dds`
- `.hdr`

Each of these formats will be converted internally for optimal GPU use. Whether you're importing hand-painted textures or high dynamic range (HDR) images, the engine will handle compression and preparation automatically.

---

## Texture Compression

To reduce memory usage and improve performance, Grindstone automatically compresses textures using GPU-friendly formats:

- **BC1**: for images with no alpha channel (RGB)
- **BC3**: for images with alpha (RGBA)
- **BC6H**: for **HDR** images only

Compression is based on the number of components in the image and whether it contains HDR data.

---

## Mipmap Generation

All imported textures will have **mipmaps** generated automatically. Mipmaps are smaller versions of the texture used when the object is far away or minified, improving performance and reducing aliasing.

This makes textures scale cleanly at all distances without additional setup.

---

## Cubemap Detection

Grindstone automatically detects **cubemap textures** during import. If the texture’s aspect ratio is exactly **3:4**, it will be flagged as a cubemap (a panoramic layout, often used to render skies).

This detection removes the need for manual tagging and ensures the texture is ready for cubemap rendering out-of-the-box.

---

## Using Textures in Materials

Once imported, TextureAssets can be assigned to samplers inside **Materials**. The material’s shader (via `GraphicsPipelineSet`) defines which textures are expected — such as `albedoTexture`, `normalTexture`, or `metallicMap` — and the editor allows you to drag and drop the corresponding texture assets.

---

## Implementation Details

Under the hood, all texture assets in Grindstone are **converted and stored as DDS files**. DDS (DirectDraw Surface) is a GPU-optimized format that supports mipmaps, cube maps, and all required compression modes.

This provides fast loading and reliable rendering behavior across platforms.

---

## Future Work

Several upcoming features are planned to give developers more control over texture importing:

- **Custom Sampler Settings**: Define wrap mode (Repeat, Clamp, Border), filtering, and LOD bias per texture.
- **Manual Compression Selection**: Choose compression formats based on usage (e.g., BC5 for normal maps, BC1 for roughness/specular).
- **Mipmap Generation Toggle**: Option to disable mipmaps for UI or special cases.
- **Texture Type Selection**: Explicitly set texture type during import:
  - 1D
  - 2D
  - 3D
  - 1D Array
  - 2D Array
  - 3D Array
  - Cubemap

These additions will help fine-tune performance, visual quality, and storage size — particularly for larger or more complex projects.

Other improvements include faster importing, especially of HDR images.