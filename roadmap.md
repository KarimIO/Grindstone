#Engine Roadmap:

This is likely outdated by the time you read this. For a more updated roadmap, check the [Trello Page](https://trello.com/b/hUDxCqfw/grindstone)

##Modules:

###Windowing
 - Win32
 - X11
 - Cocoa

###Graphics
 - OpenGL 3.3+
 - DirectX 10/11
 - DirectX 12
 - Vulkan
 - Metal

###Audio
Audio should use [SDL Audio](http://libsdl.org/) simply because while Windows Audio is easy, Linux audio is a huge pain, and all solutions are buggy. Eventually, FMOD and WWise should be provided as alternate solutions for developers.

Originally, streaming and playing audio, as well as handling microphone input and audio modifiers are all that are required.

Eventually, another library will be created to include audio synthesis and editing software. This will be importable using an advanced audio DLL.

###User Interface
We will be using LibRocket as a User Interface.

###Physics Library
Currently using bullet physics, but we need to work a lot more on integrating it properly.

###Artificial Intelligence and Pathfinding
Behavior Trees and dynamic AI should be implemented and transfered between entities simply. It should also be easy to access the system using "sense" inputs - could these be components? Finally, multiple modes of pathfinding should be implemented here, as strategy games might be better off with hex systems, and some may opt for the simpler node path approach. Crowd simulation should also be easily implemented for the user by allowing either inputs or direct access to other AI. Squads and other inter-ai systems should also be implemented, including relationship trees. We should probably have a tool for these ultimately.

##Engine specific features
_All these are planned for LATER_

###Engine Only: Internal IDE
Includes Blueprint, Scripting and Programming. Features to be discussed later. Active DLL reloading needs to be used.

###Engine Only: Debugging
Have a debug module that allows test bench features as well as code highlighting from in-engine.

###Engine Only: Sculpting
Have a debug module that allows test bench features as well as code highlighting from in-engine.

###Engine Only: Material Editing
Have a debug module that allows test bench features as well as code highlighting from in-engine.

###Engine Only: Image Editing
Image Editing from within the game allows for rapid testing. Features to be discussed later.

##Techniques
###Graphics Pipelines
Implement multiple Graphics Pipelines for a range of uses, as different games will be most optimized with different pipelines.
 - Forward
 - Forward+
 - Standard Deferred
 - Tile-Based Forward
 - Tile-Based Deferred
 - Cluster-Based Deferred
 
###Reflections
Reflections will be integrated with both SSR and Parallax-Based Cubemaps. Some Cubemaps may be generated and runtime, but this is option-added. Cubemap sizes should be optional, and geometry should have options for conditions to be rendered at.

###Alpha Stippling for Deferred
On far away objects, transparent objects should be alpha-masked.

###Multiple Material Workflows
The system should already be very customizable but default rendering techniques should include Metalness and Specular Workflows for PBR as well as the traditional shading techniques.

###Megatextures?
Should we implement megatextures somehow? It seems to have many benefits, though it may be difficult.

###CPU and GPU Particle Systems
Also, soft particles

###A range of Post-Processing techniques
Should we use a GLFX sort of system, or have many prepared techniques? Either way, we should support HDR Lighting, Film Grain, Motion Blur and many other features.

###Customizeable G-Buffer
Many games won't need motion blur, and many other techniques used in fully "realistic" games.