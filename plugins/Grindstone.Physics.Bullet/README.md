# Physics System (Bullet)

This plugin is developed by the Grindstone Foundation. It's designed to physically simulate objects moving in 3d space.

## Registered Items

### BoxCollider (Component)

Box Collider Components allow entities to take the physical shape of a box (to move through 3d space and interact with other objects physically as such).

### SphereCollider (Component)

Sphere Collider Components allow entities to take the physical shape of a sphere (to move through 3d space and interact with other objects physically as such).

### PlaneCollider (Component)

Plane Collider Components allow entities to take the physical shape of a flat plane (to move through 3d space and interact with other objects physically as such).

### CapsuleCollider (Component)

Capsule Collider Components allow entities to take the physical shape of a capsule (to move through 3d space and interact with other objects physically as such).

### Rigidbody (Component)

Rigidbody Components allow the above colliders to move as a 3d physical objects, affected by forces.

### Physics (WorldContext)

The physics worldcontext is a registry that connects to all rigidbodies and colliders. There is one per WorldContextSet.

### Physics (System)

The physics system simulates physical movement in all objects in any worldcontexts
