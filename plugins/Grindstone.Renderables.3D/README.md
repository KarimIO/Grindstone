# 3D Mesh Assets and Components

This plugin is developed by the Grindstone Foundation. It's designed to allow support meshes and mesh rendering.

## Registered Items

### Mesh3d (Asset Importer)

The mesh3d asset contains 3d points in space, designed to be combined into triangles to represent 3d surfaces. Each point usually contains positional data, normal data (directions of the associated surfaces), UV data (how textures can be used on top of the meshes).

### Mesh (Component)

This component allows an artist to associate a Mesh with an entity.

### MeshRenderer (Component)

This component provides material data to be used to render the meshes.

### Mesh3dRenderer (Asset Renderer)

The mesh 3d renderer allows the application to draw 3d MeshComponents using their MeshRendererComponent. It contains all the logic for actually drawing.
