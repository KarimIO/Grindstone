#pragma once

#include <Common/HashedString.hpp>
#include <Common/Graphics/Formats.hpp>

static Grindstone::HashedString gbufferRenderPassKey = "Gbuffer";
static Grindstone::HashedString mainRenderPassKey = "Main";

static Grindstone::HashedString dofSeparationRenderPassKey = "DofSeparation";
static Grindstone::HashedString dofBlurAndCombinationRenderPassKey = "DofBlurAndCombination";

static Grindstone::HashedString lightingRenderPassKey = "Lighting";
static Grindstone::HashedString forwardLitRenderPassKey = "ForwardLit";
static Grindstone::HashedString ssaoRenderPassKey = "Ssao";
static Grindstone::HashedString shadowMapRenderPassKey = "ShadowMap";

static const Grindstone::GraphicsAPI::Format depthFormat = Grindstone::GraphicsAPI::Format::D32_SFLOAT;
static const Grindstone::GraphicsAPI::Format litHdrFormat = Grindstone::GraphicsAPI::Format::R16G16B16A16_SFLOAT;
static const Grindstone::GraphicsAPI::Format ambientOcclusionFormat = Grindstone::GraphicsAPI::Format::R8_UNORM;

