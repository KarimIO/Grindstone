#pragma once

#include <Common/HashedString.hpp>
#include <Common/Graphics/Formats.hpp>

static Grindstone::HashedString gbufferRenderPassKey = "GbufferRenderPass";
static Grindstone::HashedString mainRenderPassKey = "MainRenderPass";

static Grindstone::HashedString dofSeparationRenderPassKey = "DofSeparationRenderPass";
static Grindstone::HashedString dofBlurAndCombinationRenderPassKey = "DofBlurAndCombinationRenderPass";

static Grindstone::HashedString lightingRenderPassKey = "LightingRenderPass";
static Grindstone::HashedString forwardLitRenderPassKey = "ForwardLitRenderPass";
static Grindstone::HashedString ssaoRenderPassKey = "SsaoRenderPass";
static Grindstone::HashedString shadowMapRenderPassKey = "ShadowMapRenderPass";

static const Grindstone::GraphicsAPI::Format depthFormat = Grindstone::GraphicsAPI::Format::D32_SFLOAT;
static const Grindstone::GraphicsAPI::Format litHdrFormat = Grindstone::GraphicsAPI::Format::R16G16B16A16_SFLOAT;
static const Grindstone::GraphicsAPI::Format ambientOcclusionFormat = Grindstone::GraphicsAPI::Format::R8_UNORM;

