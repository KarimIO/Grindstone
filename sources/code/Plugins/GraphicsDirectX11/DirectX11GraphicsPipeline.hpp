#pragma once

#include "../GraphicsCommon/GraphicsPipeline.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <d3d11.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11GraphicsPipeline : public GraphicsPipeline {
		public:
			DirectX11GraphicsPipeline(GraphicsPipelineCreateInfo ci);
			virtual ~DirectX11GraphicsPipeline() override;
		public:
			virtual void Bind() override;
		private:
			ID3D11RasterizerState* rasterizer_state_;
			ID3D11InputLayout* layout_;
			ID3D11VertexShader* vertex_shader_;		// the vertex shader
			ID3D11PixelShader* pixel_shader_;		// the fragment shader
		};
	}
}