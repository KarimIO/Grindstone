#pragma once

#include <Common/HashedString.hpp>


static Grindstone::HashedString gbufferRenderPassKey = "GbufferRenderPass";
static Grindstone::HashedString mainRenderPassKey = "MainRenderPass";

static Grindstone::HashedString dofSeparationRenderPassKey = "DofSeparationRenderPass";
static Grindstone::HashedString dofBlurAndCombinationRenderPassKey = "DofBlurAndCombinationRenderPass";

static Grindstone::HashedString lightingRenderPassKey = "LightingRenderPass";
static Grindstone::HashedString forwardLitRenderPassKey = "ForwardLitRenderPass";
static Grindstone::HashedString ssaoRenderPassKey = "SsaoRenderPass";
static Grindstone::HashedString shadowMapRenderPassKey = "ShadowMapRenderPass";
