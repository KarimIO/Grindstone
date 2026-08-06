ECS: Entities {#EcsEntities}
============

\note Return to the [ECS page](@ref ECS) for an overview.

An Entity is meant to refer to one logical "Object". It is similar to a `GameObject` in the Unity Engine, or an `Actor` in Unreal Engine. Object is not an official %Grindstone term, but is used here for the sake of explanation.

Entities in %Grindstone are simply an ID that can be used to refer to one or more [Components](@ref EcsComponents). Entities should always have at least a [Transform](@ref Grindstone::TransformComponent) and [Tag](@ref Grindstone::TagComponent) component. You can use the Grindstone::ECS::Entity class to wrap an entity and more simply utilize the helper functions it has.

In reality, a logical "Object" may have many entities that relate to them. For example, a player may have a:
 - `PlayerController`, which uses inputs to affect all other player entities.
 - `PlayerCharacter`, which represents the physical position of the player.
 - `PlayerState`, which may contain the state of non-physical data, such as the player inventory or gold.
 - `PlayerUserInterface`, which allows for showing the HUD and menus for a player.
 - `PlayerWeaponHolder`, which may be provided various Components depending on what type of weapon is held, and run arbitrary logic when a button is held.

Technically, each entity would not have any data or logic associated with it directly, but instead, they are given through components, including their name through the [Tag](@ref Grindstone::TagComponent) component, position through the [Transform](@ref Grindstone::TransformComponent) component, appearance through [MeshRendererComponent](@ref Grindstone::MeshRendererComponent), and many more gameplay-specific components.
