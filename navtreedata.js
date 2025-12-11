/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Grindstone Game Engine", "index.html", [
    [ "The MIT License", "md__2github_2workspace_2LICENSE.html", null ],
    [ "Custom Asset Editor Import Guide", "md_pages_2Assets_2CustomAssetEditorGuide.html", [
      [ "Grindstone is for you.", "index.html#autotoc_md44", null ],
      [ "Building", "index.html#autotoc_md45", null ],
      [ "License", "index.html#autotoc_md46", null ],
      [ "Step 1: Create an asset importer function", "md_pages_2Assets_2CustomAssetEditorGuide.html#autotoc_md2", null ],
      [ "Step 2: Register the Asset Importer", "md_pages_2Assets_2CustomAssetEditorGuide.html#autotoc_md4", null ]
    ] ],
    [ "Custom Asset Runtime Integration Guide", "md_pages_2Assets_2CustomAssetRuntimeGuide.html", [
      [ "Step 1: Create a Custom Asset Type", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md6", null ],
      [ "Step 2: Create an Importer for It", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md8", null ],
      [ "Step 3: Register the Asset Type", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md10", null ],
      [ "Step 4: Load and Use the Asset", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md12", [
        [ "Access by AssetReference:", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md13", null ],
        [ "Access by UUID:", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md14", null ],
        [ "Access by Address (Hashed String):", "md_pages_2Assets_2CustomAssetRuntimeGuide.html#autotoc_md15", null ]
      ] ]
    ] ],
    [ "Grindstone Asset System Overview", "md_pages_2Assets_2Overview.html", [
      [ "Purpose", "md_pages_2Assets_2Overview.html#autotoc_md19", null ],
      [ "Core Components", "md_pages_2Assets_2Overview.html#autotoc_md21", [
        [ "Asset", "md_pages_2Assets_2Overview.html#autotoc_md22", null ],
        [ "AssetReference<T>", "md_pages_2Assets_2Overview.html#autotoc_md23", null ],
        [ "AssetImporter", "md_pages_2Assets_2Overview.html#autotoc_md24", null ],
        [ "AssetManager", "md_pages_2Assets_2Overview.html#autotoc_md25", null ]
      ] ],
      [ "Extensibility", "md_pages_2Assets_2Overview.html#autotoc_md27", null ],
      [ "Loaders", "md_pages_2Assets_2Overview.html#autotoc_md29", [
        [ "FileAssetLoader", "md_pages_2Assets_2Overview.html#autotoc_md30", null ],
        [ "AssetPackLoader", "md_pages_2Assets_2Overview.html#autotoc_md31", null ]
      ] ],
      [ "Supported Assets", "md_pages_2Assets_2Overview.html#autotoc_md33", null ],
      [ "Usage Patterns", "md_pages_2Assets_2Overview.html#autotoc_md35", [
        [ "Load an asset reference:", "md_pages_2Assets_2Overview.html#autotoc_md36", [
          [ "Access by AssetReference:", "md_pages_2Assets_2Overview.html#autotoc_md37", null ],
          [ "Access by UUID:", "md_pages_2Assets_2Overview.html#autotoc_md38", null ],
          [ "Access by Address (Hashed String):", "md_pages_2Assets_2Overview.html#autotoc_md39", null ]
        ] ]
      ] ],
      [ "Notes", "md_pages_2Assets_2Overview.html#autotoc_md41", null ]
    ] ],
    [ "Building the Project", "md_pages_2BuildingTheProject.html", null ],
    [ "Plugin System 2.0 Specification", "md_pages_2Plugins_2NewDesign.html", [
      [ "Plugin Manifests", "md_pages_2Plugins_2NewDesign.html#autotoc_md49", null ],
      [ "Plugin Registry", "md_pages_2Plugins_2NewDesign.html#autotoc_md51", null ],
      [ "Plugin Folder Structure", "md_pages_2Plugins_2NewDesign.html#autotoc_md53", [
        [ "Plugin Metadata", "md_pages_2Plugins_2NewDesign.html#autotoc_md55", null ]
      ] ]
    ] ],
    [ "Grindstone Plugin System Roadmap", "md_pages_2Plugins_2Overview.html", [
      [ "Plugin List", "md_pages_2Plugins_2Overview.html#autotoc_md58", null ],
      [ "Plugin Implementation", "md_pages_2Plugins_2Overview.html#autotoc_md60", [
        [ "Example 1: Runtime Plugin", "md_pages_2Plugins_2Overview.html#autotoc_md61", null ],
        [ "Example 2: Editor Plugin", "md_pages_2Plugins_2Overview.html#autotoc_md62", null ]
      ] ],
      [ "Future Work", "md_pages_2Plugins_2Overview.html#autotoc_md64", null ]
    ] ],
    [ "DeferredRenderer", "md_pages_2Rendering_2DeferredRenderer.html", [
      [ "What is Deferred Rendering?", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md67", null ],
      [ "Summary of Render Stages", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md69", [
        [ "Opaque Geometry Pass", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md70", null ],
        [ "Lighting Pass", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md71", null ],
        [ "Unlit, Sky, and Transparent Passes", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md72", null ],
        [ "Post-Processing", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md74", null ],
        [ "Final Output", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md75", null ]
      ] ],
      [ "Future Work", "md_pages_2Rendering_2DeferredRenderer.html#autotoc_md76", null ]
    ] ],
    [ "Materials", "md_pages_2Rendering_2Materials.html", [
      [ "The Role of Materials", "md_pages_2Rendering_2Materials.html#autotoc_md79", null ],
      [ "Anatomy of a Material in Grindstone", "md_pages_2Rendering_2Materials.html#autotoc_md81", [
        [ "Shader Reference (GraphicsPipelineSet)", "md_pages_2Rendering_2Materials.html#autotoc_md82", null ],
        [ "Material Properties", "md_pages_2Rendering_2Materials.html#autotoc_md83", null ],
        [ "Material Resources (Textures and Samplers)", "md_pages_2Rendering_2Materials.html#autotoc_md84", null ]
      ] ],
      [ "Editing Materials in the Grindstone Editor", "md_pages_2Rendering_2Materials.html#autotoc_md86", null ],
      [ "Applying Materials in a Scene", "md_pages_2Rendering_2Materials.html#autotoc_md88", null ],
      [ "Runtime Material Editing", "md_pages_2Rendering_2Materials.html#autotoc_md90", null ],
      [ "Creating and Importing Materials", "md_pages_2Rendering_2Materials.html#autotoc_md92", null ],
      [ "Validation and Error Handling", "md_pages_2Rendering_2Materials.html#autotoc_md94", null ],
      [ "Best Practices", "md_pages_2Rendering_2Materials.html#autotoc_md96", null ],
      [ "JSON-Based Storage (For Developers)", "md_pages_2Rendering_2Materials.html#autotoc_md98", null ]
    ] ],
    [ "Meshes", "md_pages_2Rendering_2Meshes.html", [
      [ "Technical Details", "md_pages_2Rendering_2Meshes.html#autotoc_md100", null ]
    ] ],
    [ "Grindstone Rendering Overview", "md_pages_2Rendering_2Overview.html", [
      [ "What Is a Renderer?", "md_pages_2Rendering_2Overview.html#autotoc_md103", null ],
      [ "Asset Renderers and Render Queues", "md_pages_2Rendering_2Overview.html#autotoc_md105", null ],
      [ "Materials, Meshes, and GraphicsPipelineSets", "md_pages_2Rendering_2Overview.html#autotoc_md107", null ],
      [ "Post-Processing", "md_pages_2Rendering_2Overview.html#autotoc_md109", null ],
      [ "Render Hardware Interface (RHI)", "md_pages_2Rendering_2Overview.html#autotoc_md111", null ],
      [ "Future Work", "md_pages_2Rendering_2Overview.html#autotoc_md113", null ]
    ] ],
    [ "Shader Pipeline System Documentation", "md_pages_2Rendering_2PipelineSets.html", [
      [ "Overview", "md_pages_2Rendering_2PipelineSets.html#autotoc_md115", null ],
      [ "File Types", "md_pages_2Rendering_2PipelineSets.html#autotoc_md116", null ],
      [ "Core Constructs", "md_pages_2Rendering_2PipelineSets.html#autotoc_md117", [
        [ "include", "md_pages_2Rendering_2PipelineSets.html#autotoc_md118", null ],
        [ "pipelineSet and computeSet", "md_pages_2Rendering_2PipelineSets.html#autotoc_md120", null ],
        [ "configuration", "md_pages_2Rendering_2PipelineSets.html#autotoc_md122", null ],
        [ "pass", "md_pages_2Rendering_2PipelineSets.html#autotoc_md124", null ],
        [ "Inheritance", "md_pages_2Rendering_2PipelineSets.html#autotoc_md126", null ]
      ] ],
      [ "Pipeline Configuration Keywords", "md_pages_2Rendering_2PipelineSets.html#autotoc_md128", [
        [ "Shader Entry Points", "md_pages_2Rendering_2PipelineSets.html#autotoc_md129", null ],
        [ "renderQueue", "md_pages_2Rendering_2PipelineSets.html#autotoc_md131", null ],
        [ "properties", "md_pages_2Rendering_2PipelineSets.html#autotoc_md133", null ],
        [ "Attachments", "md_pages_2Rendering_2PipelineSets.html#autotoc_md134", null ],
        [ "Depth Bias", "md_pages_2Rendering_2PipelineSets.html#autotoc_md135", null ],
        [ "Depth Write", "md_pages_2Rendering_2PipelineSets.html#autotoc_md136", null ],
        [ "Depth Test", "md_pages_2Rendering_2PipelineSets.html#autotoc_md137", null ],
        [ "Depth Clamp", "md_pages_2Rendering_2PipelineSets.html#autotoc_md138", null ],
        [ "Cull Mode", "md_pages_2Rendering_2PipelineSets.html#autotoc_md139", null ],
        [ "Depth Compare Operation", "md_pages_2Rendering_2PipelineSets.html#autotoc_md140", null ],
        [ "Blending", "md_pages_2Rendering_2PipelineSets.html#autotoc_md141", null ]
      ] ],
      [ "Shaders", "md_pages_2Rendering_2PipelineSets.html#autotoc_md143", [
        [ "shaderBlock", "md_pages_2Rendering_2PipelineSets.html#autotoc_md144", null ],
        [ "requiresBlocks", "md_pages_2Rendering_2PipelineSets.html#autotoc_md145", null ],
        [ "shaderHlsl", "md_pages_2Rendering_2PipelineSets.html#autotoc_md147", null ]
      ] ],
      [ "Example 1: Graphics Pipeline", "md_pages_2Rendering_2PipelineSets.html#autotoc_md149", null ],
      [ "Example 2: Compute Pipeline", "md_pages_2Rendering_2PipelineSets.html#autotoc_md151", null ]
    ] ],
    [ "Textures", "md_pages_2Rendering_2Textures.html", [
      [ "What Is a Texture Asset?", "md_pages_2Rendering_2Textures.html#autotoc_md154", null ],
      [ "Supported Image Formats", "md_pages_2Rendering_2Textures.html#autotoc_md156", null ],
      [ "Texture Compression", "md_pages_2Rendering_2Textures.html#autotoc_md158", null ],
      [ "Mipmap Generation", "md_pages_2Rendering_2Textures.html#autotoc_md160", null ],
      [ "Cubemap Detection", "md_pages_2Rendering_2Textures.html#autotoc_md162", null ],
      [ "Using Textures in Materials", "md_pages_2Rendering_2Textures.html#autotoc_md164", null ],
      [ "Implementation Details", "md_pages_2Rendering_2Textures.html#autotoc_md166", null ],
      [ "Future Work", "md_pages_2Rendering_2Textures.html#autotoc_md168", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"Allocators_8hpp_source.html",
"classGrindstone_1_1Camera.html",
"md_pages_2Rendering_2DeferredRenderer.html",
"structGrindstone_1_1Reflection_1_1TypeDescriptor__Float4.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';