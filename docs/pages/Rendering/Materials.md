# Materials

When you admire the worn paint on a metal door or the shimmer of water in a game, you're seeing the result of carefully crafted **materials**. In **Grindstone**, materials control how surfaces appear under lighting and effects by linking together shaders, textures, and property values. This system allows both technical and creative users to define the look of everything from gritty terrain to sci-fi armor.

---

## The Role of Materials

Materials don't define *shape* — that’s what meshes do — but they define *how light interacts with surfaces*. This includes everything from basic colors and texture maps to more advanced features like transparency, normal mapping, or emissive glow.

In Grindstone, materials are flexible, asset-driven objects that can be previewed, edited, and reused throughout your scenes.

---

## Anatomy of a Material in Grindstone

Each material is composed of three primary elements:

### 1. Shader Reference (GraphicsPipelineSet)

Each material points to a **GraphicsPipelineSet**, Grindstone's version of a shader configuration. This defines:

- Which GPU shaders to use
- Render settings (blending, culling, depth testing, etc.)
- Available **parameters** and **resources**
- UI behavior (via shader-side definitions)

The pipeline set ensures all materials using it share consistent behavior, while still allowing individual customization through properties.

### 2. Material Properties

These are values passed into the shader to influence rendering logic. Examples include:

- `Color`: a float4 that tints the surface
- `Roughness`: a float controlling how matte or glossy the surface looks
- `UVScale`: a float2 for controlling texture tiling

Each parameter is defined by the GraphicsPipelineSet, and supports various types including float, int, vectors, booleans, and arrays.

### 3. Material Resources (Textures and Samplers)

Resources are external assets such as:

- **Textures** – e.g., diffuse, normal, roughness maps
- **Samplers** – specifying how textures are filtered, repeated, or clamped

These provide surface detail and variation, turning a flat red color into cracked paint or rough stone.

---

## Editing Materials in the Grindstone Editor

Materials are fully editable in the **Inspector Panel** once selected in the **Asset Browser**. The interface provides:

- **Shader reference** at the top (via GraphicsPipelineSet)
- **Editable property fields** and **resource pickers** based on the shader definition

Fields are automatically generated depending on the property type:

- Floats and vectors appear as sliders or number boxes
- Booleans use checkboxes
- Textures and samplers have asset selectors

This editor-driven UI ensures artists and designers can work efficiently without needing to understand shader code.

---

## Applying Materials in a Scene

Materials are applied through a combination of components:

- **Mesh3dAsset**: contains the 3D model, including submeshes
- **MeshComponent**: references the mesh asset
- **MeshRendererComponent**: assigns an **array of materials** to be used when rendering the mesh

Each submesh in the model has a **material index**, which selects the appropriate material from the array. This allows you to apply different materials to different parts of the same mesh — for example, wood for the handle, steel for the blade.

---

## Runtime Material Editing

Developers can create and customize materials at runtime using the scripting API:

```cpp
auto materialInstance = MaterialAsset.Clone();
MaterialParameter<float> param = materialInstance.GetParameter("Roughness");
materialInstance.Set(4.0f);
materialInstance.SetFloat("Roughness", 4.0f);
```

This is useful for features like changing damage states, animating materials, or adjusting visuals based on gameplay events.

---

## Creating and Importing Materials

Materials can be:

- **Created manually in the editor** through the Asset Browser
- **Imported automatically** with model files like `.fbx`, `.obj`, etc. via the **Model Importer**, which applies a default `GraphicsPipelineSet` and attempts to assign matching textures

Once imported, you can replace or customize the material as needed.

---

## Validation and Error Handling

While Grindstone currently imports all materials, **future versions will validate shader compatibility and usage**. If a material fails to import (e.g., due to a missing shader), it will appear in the **AssetWarnings** panel, and:

- The last known working version of the material will be used if available
- A fallback material will be substituted if not

This helps keep projects stable while alerting developers to issues needing attention.

---

## Best Practices

- **Use PascalCase for material names** (e.g., `SteelPlate`, `AlienSkin`)
- **Avoid duplicating similar materials** — reusing shared assets helps performance and maintainability
- **Clean up unused materials**. While they won’t be included in final builds, they can still slow down the editor and clutter your workspace.

---

## JSON-Based Storage (For Developers)

Although not exposed to artists, materials in Grindstone are stored as JSON files. This allows them to be:

- Easily serialized
- Version controlled
- Debugged manually if needed

Here’s a basic example:

```json
{
    "name": "Column B",
    "shader": "ad5ad34e-2017-487d-b2c3-489e26e63b3e",
    "parameters": {
        "color": [1, 1, 1, 1]
    },
    "samplers": {
        "diffuseTexture": "524af8b0-e993-4d92-8389-622e48b5976e",
        "normalTexture": "3b8f4c07-e8cf-4bcd-b325-8b8e31d35aeb",
        "roughnessTexture": "71ac2fa0-bdde-4fb8-9ec4-2fb66f1f1d8b"
    }
}
```

Most users will never need to edit this directly, as all material editing is done through the editor UI.
