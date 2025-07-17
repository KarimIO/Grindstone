#pragma once

#include <Common/HashedString.hpp>
#include <Common/Graphics/Formats.hpp>

static Grindstone::ConstHashedString gbufferRenderPassKey = "Gbuffer";
static Grindstone::ConstHashedString geometryOpaqueRenderPassKey = "GeometryOpaque";
static Grindstone::ConstHashedString geometryUnlitRenderPassKey = "GeometryUnlit";
static Grindstone::ConstHashedString geometrySkyRenderPassKey = "GeometrySky";
static Grindstone::ConstHashedString geometryTransparentRenderPassKey = "GeometryTransparent";
static Grindstone::ConstHashedString mainRenderPassKey = "Main";

static Grindstone::ConstHashedString dofSeparationRenderPassKey = "DofSeparation";
static Grindstone::ConstHashedString dofBlurAndCombinationRenderPassKey = "DofBlurAndCombination";

static Grindstone::ConstHashedString lightingRenderPassKey = "Lighting";
static Grindstone::ConstHashedString forwardLitRenderPassKey = "ForwardLit";
static Grindstone::ConstHashedString ssaoRenderPassKey = "Ssao";
static Grindstone::ConstHashedString ssaoBlurRenderPassKey = "Ssao Blur";
static Grindstone::ConstHashedString shadowMapRenderPassKey = "ShadowMap";

static const Grindstone::GraphicsAPI::Format depthFormat = Grindstone::GraphicsAPI::Format::D32_SFLOAT;
static const Grindstone::GraphicsAPI::Format litHdrFormat = Grindstone::GraphicsAPI::Format::R16G16B16A16_SFLOAT;
static const Grindstone::GraphicsAPI::Format ambientOcclusionFormat = Grindstone::GraphicsAPI::Format::R8_UNORM;

