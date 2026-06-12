# Navigation Meshes

This plugin is developed by the Grindstone Foundation. It's designed to allow navigation through a 3d enironment using Navigation Meshes. This system
is built on the Recast library. It works by taking in the collider geometry as triangles. Then triangles are voxelized (turned into cubes similar to a
minecraft-style environment) which simplifies processing. Finally, the navigation mesh is created on top of the voxelized geometry. All of these steps
are provided with the Recast library. Grindstone takes the physics objects and passes it to the Recast system, glues these Recast functionality together.

Then, the actual navigation processing using the Navigation Meshes is built on top of Raycast's Detour system.

## Registered Items

### NavigationMesh (Component)

Navigation Mesh Components manage the Navigation Mesh for a scene and chooses the NavAgents for which the actual NavigationMeshes will generate for.

### NavigationAgent (Component)

This component moves a gameobject towards its target point.

### NavigationMesh (WorldContext)

The physics worldcontext is a registry that connects to all rigidbodies and colliders. There is one per WorldContextSet.
