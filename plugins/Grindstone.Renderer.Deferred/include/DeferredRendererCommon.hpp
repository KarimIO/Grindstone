#pragma once

#include <Common/HashedString.hpp>
#include <Common/Graphics/Formats.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>

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


Grindstone::ConstHashedString attachmentNameAlbedo = "Grindstone.Gbuffer.Albedo";
Grindstone::ConstHashedString attachmentNameNormal = "Grindstone.Gbuffer.Normal";
Grindstone::ConstHashedString attachmentNameSpecularRoughness = "Grindstone.Gbuffer.SpecularRoughness";
Grindstone::ConstHashedString attachmentNameDepthStencil = "Grindstone.Gbuffer.DepthStencil";
Grindstone::ConstHashedString attachmentNameLighting = "Grindstone.Lighting";

Grindstone::Renderer::RenderGraph::ImageResource attachmentAlbedo{ .format = Grindstone::GraphicsAPI::Format::R8G8B8_UNORM };
Grindstone::Renderer::RenderGraph::ImageResource attachmentNormal{ .format = Grindstone::GraphicsAPI::Format::R16G16B16A16_SNORM };
Grindstone::Renderer::RenderGraph::ImageResource attachmentSpecularRoughness{ .format = Grindstone::GraphicsAPI::Format::R8G8B8_UNORM };
Grindstone::Renderer::RenderGraph::ImageResource attachmentDepthStencil{ .format = depthFormat };
Grindstone::Renderer::RenderGraph::ImageResource attachmentlighting{ .format = litHdrFormat };

Grindstone::GraphicsAPI::VertexInputLayout vertexLightPositionLayout = Grindstone::GraphicsAPI::VertexInputLayoutBuilder().AddBinding(
	{ 0, 2 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexPosition",
				0,
				Grindstone::GraphicsAPI::Format::R32G32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		}
).Build();
