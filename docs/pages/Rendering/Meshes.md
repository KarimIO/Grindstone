
# Meshes

    ! This article is a stub, you can help by expanding it.

Meshes in computer graphics are three-dimensional points in space that are connected into a group of shapes, which define the shape of an object. In Grindstone, triangles are stored in `MeshAsset`s.

Meshes can be used in a scene with `MeshComponent`s, but will not show a visible result without `MeshRendererComponents`s, which reference [Materials](Materials.md).

## Technical Details

Meshes are built using *vertices* and *indices*.

A vertex buffer contains, for each point in space, at least some of the following parameters:
| Parameter             | Description                                   |
| --------------------- | --------------------------------------------- |
| Position              | The position (x, y, z) of the point in three dimensional space. |
| Normals               | The direction perpendicular (90 degrees away) from this face. |
| Tangents              | The direction parallel (along) this face. Used for normal mapping. |
| TexCoord0             | The 2d coordinates (x, y) that will be used to wrap a texture onto the surface. |
| TexCoord1             | Additional texture coordinates, often used for lightmaps. |

An index buffer is a list of indices, or numbers that specify the orders of the vertices in the vertex buffer. Usually used so that three indices refer to one triangle, so for example, `0 1 2 1 2 4` can provide two triangles with shared verices that may be used to build a rectangle.
