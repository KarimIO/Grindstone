#include "../pch.h"
#include "DirectX12GraphicsPipeline.hpp"
#include "DirectX12RenderPass.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12Format.hpp"
#include "DirectX12UniformBuffer.hpp"
#include "DirectX12Texture.hpp"

#include <d3d12.h>
#include <d3dx12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		const char* dx12_semantic_names[] = {"POSITION", "COLOR", "TEXCOORD", "TEXCOORD", "NORMAL", "TANGENT", ""};
		DXGI_FORMAT dx12_color_format[] = {
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

		DXGI_FORMAT dx12_vert_format[] = {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT
		};

		/*
		DXGI_FORMAT color_formats
			R8 = 0,
			R8G8,
			R8G8B8,
			R8G8B8A8,

			R10G10B10A2,

			R16,
			R16G16,
			R16G16B16,
			R16G16B16A16,
			R32G32B32,
			R32G32B32A32,

			RGB_DXT1,
			RGBA_DXT1,
			RGBA_DXT3,
			RGBA_DXT5,

			SRGB_DXT1,
			SRGB_ALPHA_DXT1,
			SRGB_ALPHA_DXT3,
			SRGB_ALPHA_DXT5,
			--

		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		*/
		DirectX12GraphicsPipeline::DirectX12GraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			auto &device = DirectX12GraphicsWrapper::get().device_;
			HRESULT hr;

			ID3D12RootSignature* rootSignature;
			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ID3DBlob* signature;
			hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
			if (FAILED(hr)) {
				throw std::runtime_error("DirectX: Could not D3D12SerializeRootSignature!");
			}

			hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
			if (FAILED(hr)) {
				throw std::runtime_error("DirectX: Could not CreateRootSignature!");
			}
			
			// create input layout

			// The input layout is used by the Input Assembler so that it knows
			// how to read the vertex data bound to it.

			
			D3D12_INPUT_ELEMENT_DESC* inputLayout = new D3D12_INPUT_ELEMENT_DESC[ci.vertex_bindings[0].attribute_count];

			for (uint32_t i = 0; i < ci.vertex_bindings[0].attribute_count; ++i) {
				auto& attrib = ci.vertex_bindings[0].attributes[i];
				inputLayout[i].SemanticName = dx12_semantic_names[(uint16_t)attrib.usage];
				inputLayout[i].SemanticIndex = 0;
				inputLayout[i].Format = dx12_vert_format[(uint16_t)attrib.format];
				inputLayout[i].InputSlot = 0;
				inputLayout[i].AlignedByteOffset = attrib.offset;
				inputLayout[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				inputLayout[i].InstanceDataStepRate = 0;
			}

			// fill out an input layout description structure
			D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

			// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
			inputLayoutDesc.NumElements = ci.vertex_bindings[0].attribute_count;
			inputLayoutDesc.pInputElementDescs = inputLayout;

			DXGI_SAMPLE_DESC sampleDesc = {};
			sampleDesc.Count = 1;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
			psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
			psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
			psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
			psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
			psoDesc.NumRenderTargets = 1; // we are only binding one render target

			for (uint32_t i = 0; i < ci.shaderStageCreateInfoCount; ++i) {
				auto & shaderStageCreateInfo = ci.shaderStageCreateInfos[i];
				D3D12_SHADER_BYTECODE *shaderBytecode = &psoDesc.VS;
				switch (shaderStageCreateInfo.type) {
				default:
				case Grindstone::GraphicsAPI::ShaderStage::Vertex:
					shaderBytecode = &psoDesc.VS;
					break;
				case Grindstone::GraphicsAPI::ShaderStage::TesselationEvaluation:
					shaderBytecode = &psoDesc.HS;
					break;
				case Grindstone::GraphicsAPI::ShaderStage::TesselationControl:
					shaderBytecode = &psoDesc.DS;
					break;
				case Grindstone::GraphicsAPI::ShaderStage::Geometry:
					shaderBytecode = &psoDesc.GS;
					break;
				case Grindstone::GraphicsAPI::ShaderStage::Fragment:
					shaderBytecode = &psoDesc.PS;
					break;
				}
				shaderBytecode->BytecodeLength = shaderStageCreateInfo.size;
				shaderBytecode->pShaderBytecode = shaderStageCreateInfo.content;
			}

			// create the pso
			hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state_object_));
			if (FAILED(hr)) {
				throw std::runtime_error("DirectX: Could not CreateGraphicsPipelineState!");
			}
		}

		ID3D12PipelineState* DirectX12GraphicsPipeline::getPipelineState() {
			return pipeline_state_object_;
		}
	}
}