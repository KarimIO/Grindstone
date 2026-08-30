#pragma once

#include <stdint.h>

namespace Grindstone::Rendering {
	using RenderGraphOrderEvent = uint32_t;

	enum DeferredRenderGraphOrderEvent : RenderGraphOrderEvent {
		BeginRendering = 0,
		Skinning = 10, // TODO: Move outside of view rendering
		RenderingNonViewDependantShadows = 50, // TODO: Move outside of view rendering
		RenderingDirectionalShadows = 60,
		DepthPrePass = 150,
		OpaqueLitGeometry = 200,
		ScreenSpaceAmbientOcclusion = 225, //  TODO: Move After Depth PrePass, Before Geo
		Lighting = 250,
		SpecularSSR = 300,
		OpaqueUnlitGeometry = 350,
		Skyox = 400,
		Transparent = 450,
		DepthOfField = 500, // TODO: Where does tihs feature go?
	 	Bloom = 550,
		AntiAliasingEarly = 800,
		Tonemap = 850,
		AntiAliasingLater = 900,
		EndRendering = 1000
	};
}
