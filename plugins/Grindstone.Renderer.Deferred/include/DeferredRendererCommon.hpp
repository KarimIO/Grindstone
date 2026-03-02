#ifndef _DEFERRED_RENDERER_COMMON_HPP
#define _DEFERRED_RENDERER_COMMON_HPP

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

static Grindstone::ConstHashedString attachmentNameAlbedo = "Grindstone.Gbuffer.Albedo";
static Grindstone::ConstHashedString attachmentNameNormal = "Grindstone.Gbuffer.Normal";
static Grindstone::ConstHashedString attachmentNameSpecularRoughness = "Grindstone.Gbuffer.SpecularRoughness";
static Grindstone::ConstHashedString attachmentNameDepthStencil = "Grindstone.Gbuffer.DepthStencil";
static Grindstone::ConstHashedString attachmentNameLighting = "Grindstone.Lighting";
static Grindstone::ConstHashedString attachmentNameShadowDepthStencil = "Grindstone.ShadowAtlas";
static Grindstone::ConstHashedString attachmentNameOutput = "Grindstone.Renderer.Output";

static Grindstone::Renderer::ImageDescription attachmentAlbedo{ .format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };
static Grindstone::Renderer::ImageDescription attachmentNormal{ .format = Grindstone::GraphicsAPI::Format::R16G16B16A16_SNORM, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };
static Grindstone::Renderer::ImageDescription attachmentSpecularRoughness{ .format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };
static Grindstone::Renderer::ImageDescription attachmentDepthStencil{ .format = depthFormat, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::DepthStencil | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };
static Grindstone::Renderer::ImageDescription attachmentlighting{ .format = litHdrFormat, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };
static Grindstone::Renderer::ImageDescription attachmentOutput{ .format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM, .imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::TransferSrc | Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled };

static const uint32_t shadowAtlasResolution = 4096u;
static Grindstone::Renderer::ImageDescription attachmentShadowDepthStencil{
	.sizeClass = Grindstone::Renderer::ImageSizeType::Absolute,
	.width = static_cast<float>(shadowAtlasResolution),
	.height = static_cast<float>(shadowAtlasResolution),
	.format = depthFormat,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::DepthStencil | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
};

static Grindstone::GraphicsAPI::VertexInputLayout vertexLightPositionLayout = Grindstone::GraphicsAPI::VertexInputLayoutBuilder().AddBinding(
	{ 0, 2 * sizeof(float), Grindstone::GraphicsAPI::VertexInputRate::Vertex },
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

#endif
