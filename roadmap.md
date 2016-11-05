#Engine Roadmap:

##Modules:

###Windowing
 - Win32
 - X11
 - Cocoa?
 
No one on the project has a Mac so Cocoa has been postponed, but will likely be similar to X11.

###Graphics
 - OpenGL 3.3+
 - DirectX 10/11
 - DirectX 12
 - Vulkan
 - Metal
 
No one on the project has a Mac so Metal has been postponed. It is also the least important Graphics Language to us, as the target audience is PC/Linux (fewer gamers on Mac, OGL works on Mac anyways).

###Audio
Audio should use [SDL Audio](http://libsdl.org/) simply because while Windows Audio is easy, Linux audio is a huge pain, and all solutions are buggy. Originally, streaming and playing audio, as well as handling microphone input and audio modifiers are all that are required.

Eventually, another library will be created to include audio synthesis and editing software. This will be importable using an advanced audio DLL.

###User Interface
We need to decide whether to use Awesomium or other *ML parsers, or resort to making our own library.

###Physics Library
Implement Physics library wrapper information here. We're planning on using Bullet Physics unless a better option comes along.

###Geometry
A geometry DLL should be created to not distract the user with the details of importing models, etc.

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
Reflections will be integrated with both SSR and Parallax-Based Cubemaps. Some Cubemaps may be deferred, but this is option-added. Cubemap sizes should be optional, and geometry should have options for conditions to be rendered at.

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

###Support for many versions of Graphics Languages

###Depth of Field
What's the best way to implement this? Gaussian blur or dowscaled and combination in a seperate stage as DOOM does?

###Components-Based Engine
Using a Component and Entity based system allows for flexibility and extension we couldn't really manage otherwise, as well as memory access improvements.

###Flexible Input System
Using similar bind-function techniques to Unreal, we can implement a very flexible system.
