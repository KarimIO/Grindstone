# Proprietary File Formats
Grindstone uses a lot of proprietary file formats. Here's a rundown of the foramts used.

## Textures
### DDS: Texture Formats
We use DDS to wrap both uncompressed and lossy compressed textures. We automatically convert all files using the ConvertDDS tool executable, which will soon be integrated within the Converter tool executable. DDS files support all BCn Compressed Textures as well as uncompressed images, and includes MipMaps, Cubemaps, and other important feature. BCn can reduce frame time by up to 20%, and cut file sizes + memory sizes down to 12.5% or 16.6% (not including cubemaps. Using cubemaps can double file size).

BCn texture compression is discussed [here](http://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/).
The file format is described [here](https://msdn.microsoft.com/en-us/library/windows/desktop/bb943990(v=vs.85).aspx).

## Materials
### GJM: Grindstone JSON Material
The format using the following pseudocode:
```
{
	"shader": "../PATH/TO/shaderWithoutExtension",
	"albedoTexture": "Albedo.dds",
	"normalTexture": "Normal.dds",
	"roughnessTexture": "Roughness.DDS",
	"specularTexture": "Specular.DDS"
}
```

### GBM: Grindstone Binary Material
The format using the following pseudocode:
```
../PATH/TO/shaderWithoutExtension
Albedo.dds
Normal.dds
Roughness.DDS
Specular.DDS
```

## Geometry
### GMdF: Grindstone MoDel Format
**Grindstone Model Format** is the format we use for basic static and skeletal geometry.

The format uses the following pseudocode:
```
char[4] name = "GMdF";
struct ModelHeader {
	u32 numMeshes;
	u32 numVertices;
	u64 numIndices;
	u32 numMaterials;
	u16 numBones;
    u8  numMorphs;
	u16 flags;
}

struct Mesh {
	u32 NumIndices;
	u32 BaseVertex;
	u32 BaseIndex;
    struct MaterialReference {
        u8  renderpass;
        u8  pipeline;
        u16 material;
    } material;
    ptr unused; // Used in-Engine, but is here so we can memcpy it.
}[numMeshes];

struct Vertex {
	float[3] positions;
	float[3] normal;
	float[3] tangent;
	float[2] texCoord;
(If numBones > 0, this will be available):
    i16[4]   numBones;
    float[4] boneWeights;
}[numVertices];

u64 indices[numVertices];

struct BoneInfo {
    float[3] startPosition;
    float[3] endPosition;
    string   name;
}[numBones];

string MorphNames[numMorphs];
```

### GAnF: Grindstone Animation Format
**Grindstone Animation Format** is the format we use for skeletal animation.

The file format will be designed soon.

## Scenes
### JSON: Scene ASCII Files
To add new maps, simply create a new JSON file. You can change map files from settings.ini, under ```[GAME]```, just set the path of the ```defaultmap```. We have a few samples for you to choose from but you can make your own. Later, binary files will automatically be created.

To author your own, you need a root component. This needs an array of objects called ```entities```. For optimization, please create a field before it  called ```numentities```. The same can be said for ```cubemaps``` and ```numcubemaps``` for cubemap probe reflections, though cubemaps simply are a list of positions.

Entities for now take a ```name``` string (unused for now), and a ```components``` array of objects. ```scale```, ```position```, and ```angles``` are all temporarily used in them but they should eventually be put in a seperate ```COMPONENT_POSITION``` component.

Each component has an componentType string property. It also has more features depending on the type.
 * ```COMPONENT_RENDER``` takes a ```path``` string path to the model (.obj, .fbx, etc) file.
 * ```COMPONENT_LIGHT_DIRECTIONAL``` takes ```castshadow``` boolean, ```color``` 3-array of floats, ```brightness``` float, and ```sunradius``` float.
 * ```COMPONENT_LIGHT_POINT``` takes ```castshadow``` boolean, ```color``` 3-array of floats, ```brightness``` float, and ```lightradius``` float.
 * ```COMPONENT_LIGHT_SPOT``` takes ```castshadow``` boolean, ```color``` 3-array of floats, and ```brightness```, ```lightradius```, ```innerangle```, and ```outerangle``` floats.
 * ```COMPONENT_LIGHT_TERRAIN``` takes ```width```,  ```height```,  ```length``` floats, a  ```patches``` int for the number of patches in each axis, and  ```heightmap``` for the path to the heightmap.

### GScF: Grindstone Scene File
This file format hasn't been implemented yet. It's supposed to be a binary version of a json scene file, converter using the converter, but the scene loads fast enough, so it's being held off for a while.