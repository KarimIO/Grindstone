Entity-Component-System {#ECS}
============

Entity-Component-System (or ECS for short) is a popular design pattern in many games and game engines. The reason it is popular is because it can be both extensible and performant.

But many developers have a misunderstanding of how a "Pure" ECS should work, and a big part of that is due to the misconception that it is an "Entity-Component System". Rather, [Entities](@subpage EcsEntities), [Components](@subpage EcsComponents), and [Systems](@subpage EcsSystems) are three concepts that are all related.

Components are data and physical

Entities are physically just IDs, but logically, they are a grouping of components that ID can refer to.

Read More:

 - @subpage EcsEntities
 - @subpage EcsComponents
 - @subpage EcsSystems