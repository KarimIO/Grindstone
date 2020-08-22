#include "../pch.h"
#include "DirectX11GraphicsPipeline.hpp"
#include "DirectX11RenderPass.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Format.hpp"
#include "DirectX11UniformBuffer.hpp"
#include "DirectX11Texture.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>

namespace Grindstone {
	namespace GraphicsAPI {
		const char* dx11_semantic_names[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD", "NORMAL", "TANGENT", "" };
		DXGI_FORMAT dx11_color_format[] = {
			DXGI_FORMAT_R8_TYPELESS,
			DXGI_FORMAT_R8G8_TYPELESS,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R8G8B8A8_TYPELESS,

			DXGI_FORMAT_R10G10B10A2_TYPELESS,

			DXGI_FORMAT_R16_TYPELESS,
			DXGI_FORMAT_R16G16_TYPELESS,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16G16B16A16_TYPELESS,

			DXGI_FORMAT_R32G32B32_TYPELESS,
			DXGI_FORMAT_R32G32B32A32_TYPELESS,

			DXGI_FORMAT_BC1_TYPELESS,
			DXGI_FORMAT_BC1_TYPELESS,
			DXGI_FORMAT_BC2_TYPELESS,
			DXGI_FORMAT_BC3_TYPELESS,

			DXGI_FORMAT_BC1_TYPELESS,
			DXGI_FORMAT_BC1_TYPELESS,
			DXGI_FORMAT_BC2_TYPELESS,
			DXGI_FORMAT_BC3_TYPELESS,
		};

		DXGI_FORMAT dx11_vertex_format[] = {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT
		};

		DirectX11GraphicsPipeline::DirectX11GraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			auto device = DirectX11GraphicsWrapper::get().device_;

			// Do for each binding
			D3D11_INPUT_ELEMENT_DESC* input_layout_desc = new D3D11_INPUT_ELEMENT_DESC[ci.vertex_bindings[0].attribute_count];

			for (uint32_t i = 0; i < ci.vertex_bindings[0].attribute_count; ++i) {
				auto& attrib = ci.vertex_bindings[0].attributes[i];
				input_layout_desc[i].SemanticName = dx11_semantic_names[(uint16_t)attrib.usage];
				input_layout_desc[i].SemanticIndex = 0;
				input_layout_desc[i].Format = dx11_vertex_format[(uint16_t)attrib.format];
				input_layout_desc[i].InputSlot = 0;
				input_layout_desc[i].AlignedByteOffset = attrib.offset;
				input_layout_desc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				input_layout_desc[i].InstanceDataStepRate = 0;
			}

			for (uint32_t i = 0; i < ci.shaderStageCreateInfoCount; ++i) {
				auto& shaderStageCreateInfo = ci.shaderStageCreateInfos[i];

				switch (shaderStageCreateInfo.type) {
				case Grindstone::GraphicsAPI::ShaderStage::Vertex:
					if (device->CreateVertexShader(shaderStageCreateInfo.content, shaderStageCreateInfo.size, NULL, &vertex_shader_) < 0)
						throw std::runtime_error("Unable to load vertex shader!");

					if (device->CreateInputLayout(input_layout_desc, ci.vertex_bindings[0].attribute_count, shaderStageCreateInfo.content, shaderStageCreateInfo.size, &layout_) < 0)
						throw std::runtime_error("Unable to CreateInputLayout!");
					break;
				case Grindstone::GraphicsAPI::ShaderStage::Fragment:
					if (device->CreatePixelShader(shaderStageCreateInfo.content, shaderStageCreateInfo.size, NULL, &pixel_shader_) < 0)
						throw std::runtime_error("Unable to load fragment shader!");
					break;
				default:
				case Grindstone::GraphicsAPI::ShaderStage::TesselationEvaluation:
				case Grindstone::GraphicsAPI::ShaderStage::TesselationControl:
				case Grindstone::GraphicsAPI::ShaderStage::Geometry:
					throw std::runtime_error("Invalid shader!");
				}
			}

			// RASTERIZER
			D3D11_RASTERIZER_DESC wfdesc;
			ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
			wfdesc.AntialiasedLineEnable = FALSE;
			wfdesc.FillMode = D3D11_FILL_SOLID;
			wfdesc.CullMode = D3D11_CULL_NONE;
			wfdesc.DepthBias = 0;
			wfdesc.DepthBiasClamp = 0.0f;
			wfdesc.DepthClipEnable = true;
			wfdesc.FillMode = D3D11_FILL_SOLID;
			wfdesc.FrontCounterClockwise = false;
			wfdesc.MultisampleEnable = false;
			wfdesc.ScissorEnable = false;
			wfdesc.SlopeScaledDepthBias = 0.0f;
			switch (ci.cullMode) {
			case CullMode::None:
				wfdesc.CullMode = D3D11_CULL_NONE;
				break;
			case CullMode::Front:
				wfdesc.CullMode = D3D11_CULL_FRONT;
				break;
			case CullMode::Back:
				wfdesc.CullMode = D3D11_CULL_BACK;
				break;
			case CullMode::Both:
				wfdesc.CullMode = D3D11_CULL_NONE;
				break;
			}

			if (device->CreateRasterizerState(&wfdesc, &rasterizer_state_) < 0)
				throw std::runtime_error("Unable to load vertex shader!");
		}
		
		DirectX11GraphicsPipeline::~DirectX11GraphicsPipeline() {
			vertex_shader_->Release();
			pixel_shader_->Release();
		}

		void DirectX11GraphicsPipeline::Bind() {
			auto device = DirectX11GraphicsWrapper::get().device_;
			auto device_context = DirectX11GraphicsWrapper::get().device_context_;

			device_context->RSSetState(rasterizer_state_);

			device_context->VSSetShader(vertex_shader_, 0, 0);
			device_context->PSSetShader(pixel_shader_, 0, 0);
			device_context->IASetInputLayout(layout_);
		}
	}
}