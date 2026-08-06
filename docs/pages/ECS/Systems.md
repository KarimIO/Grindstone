ECS: Systems {#EcsSystems}
============

\note Return to the [ECS page](@ref ECS) for an overview.

A system is a function that runs every tick. While it can perform any functionality, it is intended to iterate
over [components](@ref EcsComponents) to do some operation on them. For example, applying transformations from the physics systems, processing animations, or rendering a scene.

The purpose of operating on all similar Components at the same time, rather than iterating on all logic related to one Entity, is that doing so is faster for the computer, as all Components will be tightly packed together, and so the computer will not need to jump to parts of memory that are far apart, and are not [cached](@ref https://www.geeksforgeeks.org/computer-science-fundamentals/cache-memory/).

## Example

Here is an example of a system that reduces your health depending on your position:

```c++
void RegenerationSystem(Grindstone::WorldContextSet& worldContextSet) {
    Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	const float deltaTime = engineCore.GetDeltaTime();
    const float healthReduceRate = 4.0f * deltaTime;

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	auto view = registry.view<Grindstone::Ai::NavAgentComponent, Grindstone::TransformComponent>();
	view.each(
		[&cxt](
			HealthComponent& healthComponent,
			const Grindstone::TransformComponent& transformComponent
		) {
            // Reduce the health to 0
            healthComponent.health = std::max(healthComponent.health - healthReduceRate, 0.0f);
		}
	);
}
```

Then in your plugin's `EntryPoint.cpp`, you can run:
 - `pluginInterface->RegisterSystem("MyGame::RegenerationSystem", RegenerationSystem);` on `InitializeModule`
 - `pluginInterface->UnregisterSystem("MyGame::RegenerationSystem");` on `ReleaseModule`.
