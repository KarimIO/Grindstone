# Editor PipelineSet Importer

This plugin is developed by the Grindstone Foundation. It's designed to allow import pipelinesets which are groups of shaders. It can render materials in a variety of configurations (depending on hardware specifications, distance to objects, etc) and settings (such as regular rendering, shadows, etc).

## Registered Items

### PSET PipelineSet (Asset Importer)

The pipelineset importer imports pipelinesets from their source file format, verifies the code, compiles the shaders, and outputs the final binary format to be used in the engine.