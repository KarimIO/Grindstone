
# Shader Pipeline System Documentation

## Overview

This pipeline system provides a declarative syntax for defining rendering pipelines and compute workloads for the Grindstone Game Engine. It supports modular reusability, inheritance, and specialization, allowing technical designers and engineers to author flexible and maintainable shader and pipeline definitions.

## File Types

- `.gpset`: A pipeline set file, which can define one or more graphics or compute pipelines.
- `.hlsl`: Embedded directly within the pipeline set files for shader code.

## Core Constructs

### `include`

```plaintext
include "path/to/another/pipelineset.gpset"
```

Used to include other pipeline set files, which may define reusable blocks, shared uniforms, functions, or base pipelines.

---

### `pipelineSet` and `computeSet`

- `pipelineSet "<Name>"` defines a graphics pipeline.
- `computeSet "<Name>"` defines a compute pipeline.

---

### `configuration`

Defines one or more configurations within a pipeline set. Each configuration can contain multiple passes (for graphics pipelines) or a single compute shader. Only one configuration should be used by a camera at a time.

Currently there is no selection mechanic but in the future, a configuration will be selected based on a list of selected tags - some static and some runtime. This will allow changing the quality or performance of an effect based on needs. For example:

| Static tags           | Description                     |
| --------------------- | ------------------------------- |
| GraphicsAPI           | `vulkan`, `opengl`, `dx12`      |
| SupportsTesselation   | `true`, `false`                 |
| Lod (Level of Detail) | Integer number (0+)             |

---

### `pass`

Defined within a configuration (graphics only). Represents a rendering pass, with all shader data and render state configuration. If a pass exists outside of a configuration, it will not be used except for inheritance.

---

### Inheritance

```plaintext
pipelineSet "X" inherits "Y" {
  ...
```

This allows a pipelineSet, computeSet, configuration, or pass to inherit from another.

---

## Pipeline Configuration Keywords

### Shader Entry Points

```plaintext
shaderEntrypoint: <shader stage> <function>
```

Specifies the main entrypoint(s) for the shader stage(s).

Shader stages include:
 - `vertex`
 - `fragment` (known as `pixel` in DirectX)
 - `geometry`
 - `tesselationEvaluation` (known as `domain shader` in DirectX)
 - `tesselationControl` (known as `hull shader` in DirectX)
 - `task` (known as `amplification` in DirectX)
 - `mesh`
 - `compute`

**NOTE**: DirectX shader stage names are not used in Grindstone except within the DirectX Render Hardware Interface.

---

### `renderQueue`

```plaintext
renderQueue: "queueName"
```

Sets the logical ordering of the rendering pass within the frame, useful for sorting transparent, opaque, or post-processing steps. See the documentation of your renderer to know which RenderQueues are accepted. If this is not used for geometry in the scene, this field is also used for discovering render passes.

---

### `properties`

This describes the way the triangle pipeline outside of the shaders work, by setting fixed-function properties used in attachments, depth testing, and culling:

```plaintext
properties {
  cull: both | back | front | none
  depthBias: false | BiasConstantFactor BiasSlopeFactor BiasClamp
  depthWrite: true | false
  depthTest: true | false
  depthClamp: false | nearValue | farValue
  depthCompareOp: never | less | equal | lessOrEqual | greater | notEqual | greaterOrEqual | always
  attachments: {
    colorMask: ColorMask
    blendPreset: BlendPreset
    blendColor: Operation FactorSrc FactorDst
    blendAlpha: Operation FactorSrc FactorDst
  }
}
```

### Attachments

| Attachment Keys | Description   |
| ------------- | ------------- |
| colorMask | Takes a combination of `r`, `g`, `b`, `a` (both lowercase and uppercase) to indicate which channels will be affected during rendering. For example, `gAb` activates green, alpha, and blue channels.
| blendPreset | Used instead of `blendColor` and `blendAlpha`. These can simplify the definition of blend states, as they have necessary common presets. |
| blendColor | Used with `blendAlpha`, but not `blendPreset`. Acceptes a BlendOperation and two BlendFactors, one for source color factor, and one for destination color factor. |
| blendAlpha | Used with `blendColor`, but not `blendPreset`. Acceptes a BlendOperation and two BlendFactors, one for source alpha factor, and one for destination color factor. |

### Depth Bias

Biases the depth of a pixel to prevent artifacts (usually shadow artifacts). Accepts either `false`, or a series of three numbers that represent a curve.

### Depth Write

A boolean which decides whether or not the fragment shader will write to the depth buffer.

### Depth Test

A boolean which decides whether or not the fragment shader will write to the depth buffer.

### Depth Clamp

If `false`, this feature is disabled, otherwise, this field takes a series of two numbers - nearValue and farValue, and clamps the depth value between them.

### Cull Mode

Determines which side of a triangle would be drawn. Front faces of a triangle are those where the positions are defined in counter-clockwise order, and back faces are in clockwise order. Culling one side of a triangle saves on rendering pixels unnecessarily.

| Cull Mode | Description   |
| ------------- | ------------- |
| `both` | Both front and back faces are hidden (nothing is drawn).
| `front` | Fronts of faces are hidden. |
| `back` | Backs of faces are hidden *(default)*. |
| `none` | Neither front nor back faces are hidden (the entire triangle is drawn). |

### Depth Compare Operation

Decides which depth values are accepted when a comparison is initiated between an existing pixel and a new one. Accepts one of: `Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always`

### Blending

Defines how two pixels drawn on top of each other are blended together. You either need to specify `blendPreset`, or both `blendColor` and `blendAlpha`.
 * `blendPreset` includes commonly used defaults, including the default of `Opaque`.
 * `blendColor` defines how the RGB channels are combined.
 * `blendAlpha` defines how the alpha channel is combined.

| `BlendPreset` | Description   |
| ------------- | ------------- |
| `opaque` | No color blending at all. <br /> `blendColor: none one one` <br /> `blendAlpha: none one one` |
| `translucent` | The most common blending when it is enabled, where the top pixel's alpha indicates how blended the two are. <br /> `blendColor: add srcAlpha oneMinusSrcAlpha` <br /> `blendAlpha: add one zero` |
| `additive` | Simply add the two values, brightening the result. <br /> `blendColor: add srcAlpha one` <br /> `blendAlpha: add one zero` |
| `multiplicative` | Simply multiple the two values, usually darkening the result. <br /> `blendColor: multiply one one` <br /> `blendAlpha: multiply one one` |
| `premultiply` | First multiplies color before blending by its alpha similarly to Translucent. <br /> `blendColor: add one oneMinusSrcAlpha` <br /> `blendAlpha: add one oneMinusSrcAlpha` |

| Blend Operation      | Description                                                                |
| -------------------- | -------------------------------------------------------------------------- |
| **None**             | No blending; source completely replaces the destination.                   |
| **Add**              | Adds the source and destination colors: `Src + Dst`.                       |
| **Subtract**         | Subtracts destination from source: `Src - Dst`.                            |
| **ReverseSubtract**  | Subtracts source from destination: `Dst - Src`.                            |
| **Minimum**          | Takes the minimum of source and destination for each component.            |
| **Maximum**          | Takes the maximum of source and destination for each component.            |
| **Zero**             | Uses zero as the blend factor; effectively removes contribution.           |
| **Source**           | Only the source color is used (equivalent to `None`).                      |
| **Destination**      | Only the destination color is kept (source discarded).                     |
| **SourceOver**       | Standard alpha blending: `Src + (1 - SrcAlpha) * Dst`.                     |
| **DestinationOver**  | `Dst + (1 - DstAlpha) * Src`.                                              |
| **SourceIn**         | `Src * DstAlpha`, only where destination exists.                           |
| **DestinationIn**    | `Dst * SrcAlpha`, only where source exists.                                |
| **SourceOut**        | `Src * (1 - DstAlpha)`, only where destination is absent.                  |
| **DestinationOut**   | `Dst * (1 - SrcAlpha)`, only where source is absent.                       |
| **SourceAtop**       | `Src * DstAlpha + Dst * (1 - SrcAlpha)`.                                   |
| **DestinationAtop**  | `Dst * SrcAlpha + Src * (1 - DstAlpha)`.                                   |
| **XOR**              | `Src * (1 - DstAlpha) + Dst * (1 - SrcAlpha)`.                             |
| **Multiply**         | Multiplies source and destination: `Src * Dst`, darkens.                   |
| **Screen**           | Inverse multiply: `1 - (1 - Src) * (1 - Dst)`, lightens.                   |
| **Overlay**          | Combines Multiply and Screen based on Dst: darkens dark, lightens light.   |
| **Darken**           | `min(Src, Dst)`, keeps the darker color.                                   |
| **Lighten**          | `max(Src, Dst)`, keeps the lighter color.                                  |
| **ColorDodge**       | Brightens destination to reflect the source.                               |
| **ColorBurn**        | Darkens destination to reflect the source.                                 |
| **HardLight**        | Screen or Multiply depending on Src brightness.                            |
| **SoftLight**        | Soft contrast blend; similar to diffuse lighting.                          |
| **Difference**       | Absolute difference: `abs(Dst - Src)`.                                     |
| **Exclusion**        | Similar to Difference but with less contrast: `Dst + Src - 2 * Dst * Src`. |
| **Invert**           | Inverts all components of the destination.                                 |
| **InvertRGB**        | Inverts only the RGB components of the destination.                        |
| **LinearDodge**      | Additive blend with clamping.                                              |
| **LinearBurn**       | Subtractive blend with clamping: `Src + Dst - 1`.                          |
| **VividLight**       | Combination of ColorDodge and ColorBurn based on Src.                      |
| **LinearLight**      | Adds or subtracts Src based on its luminance: `Dst + 2 * Src - 1`.         |
| **PinLight**         | Replaces colors depending on brightness: clamps using Src.                 |
| **HardMix**          | Result is either 0 or 1 per channel depending on threshold.                |
| **HSLHue**           | Combines hue of Src with saturation and luminance of Dst.                  |
| **HSLSaturation**    | Combines saturation of Src with hue and luminance of Dst.                  |
| **HSLColor**         | Combines hue and saturation of Src with luminance of Dst.                  |
| **HSLLuminosity**    | Combines luminance of Src with hue and saturation of Dst.                  |
| **Plus**             | Additive blend, may overflow.                                              |
| **PlusClamped**      | Additive blend clamped to \[0, 1].                                         |
| **PlusClampedAlpha** | Additive blend with alpha clamped to \[0, 1].                              |
| **PlusDark**         | Add then take the darker result.                                           |
| **Minus**            | Subtractive blend, may underflow.                                          |
| **MinusClamped**     | Subtractive blend clamped to \[0, 1].                                      |
| **Contrast**         | Increases difference from mid-gray; enhances contrast.                     |
| **InvertOVG**        | OpenVG-style inversion of destination color.                               |
| **Red**              | Only red channel of the source is used; others 0.                          |
| **Green**            | Only green channel of the source is used; others 0.                        |
| **Blue**             | Only blue channel of the source is used; others 0.                         |

| Blend Factor              | Description                                                                              |
| ------------------------- | ---------------------------------------------------------------------------------------- |
| **Zero**                  | Always 0.0 — the color contributes nothing.                                              |
| **One**                   | Always 1.0 — the color is used as-is.                                                    |
| **SrcColor**              | Multiplies by the source color: `source.rgb` or `source.rgba`.                           |
| **OneMinusSrcColor**      | Multiplies by (1 - source color): `1 - source.rgb` or `1 - source.rgba`.                 |
| **DstColor**              | Multiplies by the destination color: `dest.rgb`.                                         |
| **OneMinusDstColor**      | Multiplies by (1 - destination color): `1 - dest.rgb`.                                   |
| **SrcAlpha**              | Multiplies by the source alpha: `source.a`.                                              |
| **OneMinusSrcAlpha**      | Multiplies by (1 - source alpha): `1 - source.a`.                                        |
| **DstAlpha**              | Multiplies by the destination alpha: `dest.a`.                                           |
| **OneMinusDstAlpha**      | Multiplies by (1 - destination alpha): `1 - dest.a`.                                     |
| **ConstantColor**         | Multiplies by a user-defined constant color set with `glBlendColor`.                     |
| **OneMinusConstantColor** | Multiplies by (1 - constant color).                                                      |
| **ConstantAlpha**         | Multiplies by the alpha of the constant color.                                           |
| **OneMinusConstantAlpha** | Multiplies by (1 - constant alpha).                                                      |
| **SrcAlphaSaturate**      | Uses `min(source.a, 1 - dest.a)` for RGB; alpha is set to 1.0. Useful for fade-to-black. |
| **Src1Color**             | Multiplies by the second source color (used in dual-source blending).                    |
| **OneMinusSrc1Color**     | Multiplies by (1 - second source color).                                                 |
| **Src1Alpha**             | Multiplies by the alpha of the second source color.                                      |
| **OneMinusSrc1Alpha**     | Multiplies by (1 - alpha of the second source color).                                    |

---

## Shaders

### `shaderBlock`

Declares HLSL data that can be reused by multiple shaders. Can reference other blocks.

```hlsl
shaderBlock GsComputeWorldSpacePosition {
	requiresBlocks [
		GsComputeClipSpacePosition,
		GsInvertMatrix
	]

	shaderHlsl {
		float3 ComputeWorldSpacePosition(float4x4 projectionMatrix, float4x4 viewMatrix, float2 positionNDC, float deviceDepth) {
			float4 positionCS  = ComputeClipSpacePosition(positionNDC, deviceDepth);
			float4 hpositionWS = mul(positionCS, InvertMatrix(projectionMatrix * viewMatrix));
			return hpositionWS.xyz / hpositionWS.w;
		}
	}
}
```

### `requiresBlocks`

Specifies a list of shared **shaderBlocks** that should be injected or made available.

```plaintext
requiresBlocks [
  GsViewNormal,
  GsRendererUniform,
  ...
]
```

---

### `shaderHlsl`

Embedded HLSL source. This block supports vertex, fragment, or compute code, depending on the pipeline type. Technical designers are not expected to modify this unless they are writing or modifying shaders. Learn about how to use HLSL for Grindstone here: [Vulkan.org - HLSL in Vulkan](https://docs.vulkan.org/guide/latest/hlsl.html#_hlsl).

```plaintext
shaderHlsl {
  // HLSL code here
}
```

---

## Example 1: Graphics Pipeline

```plaintext
include "$ENGINE/pipelinesets/common/matrixTransformations.gpset"

pipelineSet "Depth of Field (Blur)" inherits "PostProcessing" {
  configuration "main" {
    pass "main" {
      shaderEntrypoint: vertex mainVertex
      shaderEntrypoint: fragment mainFragment
      renderQueue: "DofBlurAndCombination"
      properties {
        cull: back
        depthBias: false
        depthWrite: false
        depthTest: false
        depthClamp: false
        depthCompareOp: lessOrEqual
        attachments: {
          colorMask: rgba
          blendPreset: opaque
        }
      }
      shaderHlsl {
        // Vertex and fragment shader code here
      }
    }
  }
}
```

---

## Example 2: Compute Pipeline

```plaintext
include "$ENGINE/pipelinesets/common/matrixTransformations.gpset"
include "$ENGINE/pipelinesets/common/rendererUniform.gpset"

computeSet "Bloom" {
  shaderEntrypoint: compute main

  requiresBlocks [
    GsViewNormal,
    GsComputeClipSpacePosition,
    GsComputeViewSpacePosition,
    GsRendererUniform,
  ]

  shaderHlsl {
    // Compute shader code here
  }
}
```
