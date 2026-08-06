WorldContext {#WorldContext}
============

In %Grindstone, we may want to have multiple simulations exist completely disconnected from one another. We call these simulations "Worlds". This is why we have a [WorldContext](@ref Grindstone::WorldContext) system. 

## Overview

To explain, let's go from narrow to broad:
 - A [WorldContext](@ref Grindstone::WorldContext) is an abstract class, from which developers can group logic and data for a particular World. For example:
    - [NavMeshWorldContext](@ref Grindstone::Ai::NavMeshWorldContext) which allows NPCs and potentially players to navigate a world.
    - [PhysicsWorldContext](@ref Grindstone::Physics::WorldContext) which includes physics information for a world and allows all physics objects to collide and interact.
 - A [WorldContextSet](@ref Grindstone::WorldContextSet) contains an entity registry and all WorldContexts for that world.
 - The [WorldContextManager](@ref Grindstone::WorldContextManager) is a [singleton](https://en.wikipedia.org/wiki/Singleton_pattern) that contains a list of all WorldContextSets (see below), and factory functions to create WorldContexts for each WorldContextSet.

When a new WorldContext factory is registered, all existing WorldContextSets are given a new WorldContext. When a new WorldContextSet is created, all WorldContext factories are triggered to create the necessary contexts for the new World.

This allows developers of games and tools to create worlds that do not interact with each other and work independently.

## Utility

WorldContextSets are passed around for systems and in general. This allows systems and other functions to know what entities it should work on, and the world data for particular contexts (such as physics and navigation). You will need to pass these around a lot.

You also may want to use these to create your own "Manager" classes, such as an enemy spawner, or pickup director. Be aware that these will be created in ALL worlds, not just gameplay ones (as there is no difference between a gameplay world and another type of world). Typically, WorldContexts do not **do** anything, they simply group utility functions and data.
