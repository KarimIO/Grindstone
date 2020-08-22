#pragma once

#include "../GraphicsCommon/GraphicsPipeline.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12GraphicsPipeline : public GraphicsPipeline {
		public:
			DirectX12GraphicsPipeline(GraphicsPipelineCreateInfo ci);
			virtual ~DirectX12GraphicsPipeline() {};
			//VkPipeline getGraphicsPipeline();
			//VkPipelineLayout getGraphicsPipelineLayout();
		public:
			virtual void Bind() {};
			ID3D12PipelineState* getPipelineState();
		private:
			//void createShaderModule(ShaderStageCreateInfo &ci, VkPipelineShaderStageCreateInfo &out);
		private:
			ID3D12PipelineState* pipeline_state_object_;
		};
	}
}