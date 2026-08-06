ECS: Components {#EcsComponents}
============

\note Return to the [ECS page](@ref ECS) for an overview.

A component is a grouping of data and utility functions that exist on an [entity](@ref EcsEntity). It is typically operated on by [Systems](@ref EcsSystem). All components of an entity are shown in the %Grindstone Editor's Inspector panel when that entity is selected.

## Component Functionality

If components have some specific static functions, those will be used by the ECS system.

 - `static void Construct(Grindstone::WorldContextSet& worldContextSet, entt::entity entity)` is used when a component is added to an entity.
 - `static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity)` is used when a component is removed from an entity.

## Example

Here is an example of how an entity can be defined. The [MeshRendererComponent](@ref Grindstone::MeshRendererComponent) allows for rendering of [Meshes](@ref Meshes).

As you can see, it contains a vector of [Materials](@ref Materials) by which to render the submeshes of a [MeshComponent](@ref Grindstone::MeshComponent). A typical pattern is to have multiple components work together when used in a System.

Furthermore, you can see the static functions `Construct` and `Destroy`, which provide the ECS logic referenced above.

```c++
namespace Grindstone {
	struct MeshRendererComponent {
		std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>> materials;

		static void Construct(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);

		Grindstone::GraphicsAPI::Buffer* perDrawUniformBuffer = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* perDrawDescriptorSet = nullptr;

		REFLECT("MeshRenderer")
	};
}
```