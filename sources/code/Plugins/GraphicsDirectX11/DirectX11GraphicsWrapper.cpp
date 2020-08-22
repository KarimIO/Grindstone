#include "../pch.h"

#define NOMINMAX

#include <cassert>

#include <windows.h>
#include <windowsx.h>

// DirectX 11 specific headers.
#include <d3d11.h>

#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11CommandBuffer.hpp"
#include "DirectX11DepthTarget.hpp"
#include "DirectX11Descriptors.hpp"
#include "DirectX11Framebuffer.hpp"
#include "DirectX11GraphicsPipeline.hpp"
#include "DirectX11IndexBuffer.hpp"
#include "DirectX11RenderPass.hpp"
#include "DirectX11RenderTarget.hpp"
#include "DirectX11Texture.hpp"
#include "DirectX11UniformBuffer.hpp"
#include "DirectX11VertexBuffer.hpp"
#include "DirectX11VertexArrayObject.hpp"
#include "DirectX11Format.hpp"
#include <set>
#include <algorithm>
#include <array>

#include <glm/glm.hpp>

#include "Window/Win32Window.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;
const int num_buffers = 3;

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11GraphicsWrapper *DirectX11GraphicsWrapper::graphics_wrapper_ = nullptr;

		bool DirectX11GraphicsWrapper::initialize(GraphicsWrapperCreateInfo ci) {
			api_type_ = GraphicsAPIType::DirectX11;
			graphics_wrapper_ = this;
			debug_ = ci.debug;
			window_ = ci.window;

			pickPhysicalDevice();
			createLogicalDeviceAndSwapChain();

			ID3D11Texture2D* pBackBuffer;
			swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

			// use the back buffer address to create the render target
			if (FAILED(device_->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer_)))
				throw std::runtime_error("DirectX: Could not CreateRenderTargetView!");

			pBackBuffer->Release();

			unsigned int width, height;
			window_->getWindowSize(width, height);

			D3D11_TEXTURE2D_DESC depth_stencil_desc;
			ZeroMemory(&depth_stencil_desc, sizeof(D3D11_TEXTURE2D_DESC));
			depth_stencil_desc.Width = width;
			depth_stencil_desc.Height = height;
			depth_stencil_desc.MipLevels = 1;
			depth_stencil_desc.ArraySize = 1;
			depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_stencil_desc.SampleDesc.Count = 1;
			depth_stencil_desc.SampleDesc.Quality = 0;
			depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
			depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depth_stencil_desc.CPUAccessFlags = 0;
			depth_stencil_desc.MiscFlags = 0;

			//Create the Depth/Stencil View
			if (device_->CreateTexture2D(&depth_stencil_desc, NULL, &depth_stencil_buffer_) < 0)
				throw std::runtime_error("DirectX: Could not CreateTexture2D!");

			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			// Initailze the depth stencil view.
			ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

			// Set up the depth stencil view description.
			depthStencilViewDesc.Format = depth_stencil_desc.Format;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			if (device_->CreateDepthStencilView(depth_stencil_buffer_, nullptr, &depth_stencil_view_) < 0)
				throw std::runtime_error("DirectX: Could not CreateDepthStencilView!");

			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

			// Set up the description of the stencil state.
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			depthStencilDesc.StencilEnable = false;

			ID3D11DepthStencilState *depthStencilState;

			// Create the depth stencil state.
			if (device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilState) < 0)
			{
				return false;
			}

			// Set the depth stencil state.
			device_context_->OMSetDepthStencilState(depthStencilState, 1);

			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = 800;
			viewport.Height = 600;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			device_context_->RSSetViewports(1, &viewport);

			return true;
		}

		void DirectX11GraphicsWrapper::pickPhysicalDevice() {
			ComPtr<IDXGIFactory> dxgiFactory;
			if (CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory)) < 0)
				throw std::runtime_error("DirectX: Could not create dxgiFactory!");

			ComPtr<IDXGIAdapter> dxgiAdapter;

			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC dxgiAdapterDesc;
				dxgiAdapter->GetDesc(&dxgiAdapterDesc);

				// dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				if (dxgiAdapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory) {
					maxDedicatedVideoMemory = dxgiAdapterDesc.DedicatedVideoMemory;
					adapter_ = dxgiAdapter;
				}
			}

			api_version_ = "11.0";
			DXGI_ADAPTER_DESC adapterDescription;
			adapter_->GetDesc(&adapterDescription);
			std::wstring dc = adapterDescription.Description;
			const char* vendor_name = getVendorNameFromID(adapterDescription.VendorId);
			if (vendor_name == 0) {
				vendor_name_ = std::string("Unknown Vendor(") + std::to_string(adapterDescription.VendorId) + ")";
			}
			else {
				vendor_name_ = vendor_name;
			}
			adapter_name_ = std::string(dc.begin(), dc.end());
		}

		void DirectX11GraphicsWrapper::createLogicalDeviceAndSwapChain() {
			// create a struct to hold information about the swap chain
			DXGI_SWAP_CHAIN_DESC scd;

			// clear out the struct for use
			ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

			// fill the swap chain description struct
			scd.BufferCount = 1;                                    // one back buffer
			scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
			scd.OutputWindow = ((Win32Window*)window_)->getHandle();                                // the window to be used
			scd.SampleDesc.Count = 4;                               // how many multisamples
			scd.Windowed = TRUE;                                    // windowed/full-screen mode

			// create a device, device context and swap chain using the information in the scd struct
			D3D11CreateDeviceAndSwapChain(NULL, // adapter_
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				NULL,
				NULL,
				NULL,
				D3D11_SDK_VERSION,
				&scd,
				&swap_chain_,
				&device_,
				NULL,
				&device_context_);
		}

		DirectX11GraphicsWrapper::~DirectX11GraphicsWrapper() {
			if (depth_stencil_view_) {
				depth_stencil_view_->Release();
				depth_stencil_view_ = 0;
			}

			if (depth_stencil_buffer_) {
				depth_stencil_buffer_->Release();
				depth_stencil_buffer_ = 0;
			}
			
			if (swap_chain_) {
				swap_chain_->Release();
				swap_chain_ = 0;
			}

			if (backbuffer_) {
				backbuffer_->Release();
				backbuffer_ = 0;
			}

			if (device_) {
				device_->Release();
				device_ = 0;
			}

			if (device_context_) {
				device_context_->Release();
				device_context_ = 0;
			}
		}

		uint32_t DirectX11GraphicsWrapper::getImageIndex()
		{
			return uint32_t();
		}

		void DirectX11GraphicsWrapper::waitUntilIdle() {
		}

		void DirectX11GraphicsWrapper::drawCommandBuffers(uint32_t ii, CommandBuffer** commandBuffers, uint32_t commandBufferCount) {
			
		}

		ColorFormat DirectX11GraphicsWrapper::getDeviceColorFormat() {
			return ColorFormat::R8; // swapchain_format_;
		}

		DirectX11GraphicsWrapper& DirectX11GraphicsWrapper::get() {
			return *graphics_wrapper_;
		}

		void DirectX11GraphicsWrapper::adjustPerspective(float* perspective) {
			glm::mat4 *mat = (glm::mat4*)perspective;

			const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
				0, 1.0f, 0, 0,
				0, 0, 0.5f, 0,
				0, 0, 0.25f, 1.0f);

			*mat = scale * (*mat);
		}

		void DirectX11GraphicsWrapper::getSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count)
		{
		}

		static int i = 0;
		void DirectX11GraphicsWrapper::clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {

			// set the render target as the back buffer

			if (((uint8_t)mask & (uint8_t)ClearMode::Color))
				device_context_->ClearRenderTargetView(backbuffer_, clear_color);

			UINT clear_flag = (((uint8_t)mask & (uint8_t)ClearMode::Depth) ? D3D11_CLEAR_DEPTH : 0) | (((uint8_t)mask & (uint8_t)ClearMode::Stencil) ? D3D11_CLEAR_STENCIL : 0);
			device_context_->ClearDepthStencilView(depth_stencil_view_, clear_flag, clear_depth, clear_stencil);
			
			device_context_->OMSetRenderTargets(1, &backbuffer_, depth_stencil_view_);
		}

		void DirectX11GraphicsWrapper::bindVertexArrayObject(VertexArrayObject*vao) {
			vao->Bind();
		}

		void DirectX11GraphicsWrapper::bindTextureBinding(TextureBinding* b) {
			DirectX11TextureBinding* db = (DirectX11TextureBinding *)b;
			db->bind();
		}

		//==================================
		// Get Text Metainfo
		//==================================
		const char* DirectX11GraphicsWrapper::getVendorName() {
			return vendor_name_.c_str();
		}

		const char* DirectX11GraphicsWrapper::getAdapterName() {
			return adapter_name_.c_str();
		}

		const char* DirectX11GraphicsWrapper::getAPIName() {
			return "DirectX11";
		}

		const char* DirectX11GraphicsWrapper::getAPIVersion() {
			return api_version_.c_str();
		}

		//==================================
		// Creators
		//==================================
		Framebuffer* DirectX11GraphicsWrapper::createFramebuffer(FramebufferCreateInfo ci) {
			return nullptr; // return static_cast<Framebuffer*>(new DirectX11Framebuffer(ci));
		}

		RenderPass* DirectX11GraphicsWrapper::createRenderPass(RenderPassCreateInfo ci) {
			return nullptr; //return static_cast<RenderPass*>(new DirectX11RenderPass(ci));
		}

		GraphicsPipeline* DirectX11GraphicsWrapper::createGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			return static_cast<GraphicsPipeline*>(new DirectX11GraphicsPipeline(ci));
		}

		CommandBuffer* DirectX11GraphicsWrapper::createCommandBuffer(CommandBufferCreateInfo ci) {
			return nullptr; //return static_cast<CommandBuffer*>(new DirectX11CommandBuffer(ci));
		}

		VertexArrayObject* DirectX11GraphicsWrapper::createVertexArrayObject(VertexArrayObjectCreateInfo ci) {
			return static_cast<VertexArrayObject*>(new DirectX11VertexArrayObject(ci));
		}

		VertexBuffer* DirectX11GraphicsWrapper::createVertexBuffer(VertexBufferCreateInfo ci) {
			return static_cast<VertexBuffer*>(new DirectX11VertexBuffer(ci));
		}

		IndexBuffer* DirectX11GraphicsWrapper::createIndexBuffer(IndexBufferCreateInfo ci) {
			return static_cast<IndexBuffer*>(new DirectX11IndexBuffer(ci));
		}

		UniformBuffer* DirectX11GraphicsWrapper::createUniformBuffer(UniformBufferCreateInfo ci) {
			return static_cast<UniformBuffer*>(new DirectX11UniformBuffer(ci));
		}

		UniformBufferBinding* DirectX11GraphicsWrapper::createUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			return static_cast<UniformBufferBinding*>(new DirectX11UniformBufferBinding(ci));
		}

		Texture* DirectX11GraphicsWrapper::createCubemap(CubemapCreateInfo ci) {
			return nullptr; // static_cast<Texture *>(new DirectX11Texture(ci));
		}

		Texture* DirectX11GraphicsWrapper::createTexture(TextureCreateInfo ci) {
			return static_cast<Texture*>(new DirectX11Texture(ci));
		}

		TextureBinding* DirectX11GraphicsWrapper::createTextureBinding(TextureBindingCreateInfo ci) {
			return static_cast<TextureBinding*>(new DirectX11TextureBinding(ci));
		}

		TextureBindingLayout* DirectX11GraphicsWrapper::createTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			return static_cast<TextureBindingLayout*>(new DirectX11TextureBindingLayout(ci));
		}

		RenderTarget* DirectX11GraphicsWrapper::createRenderTarget(RenderTargetCreateInfo* ci, uint32_t rc, bool cube) {
			return nullptr; //return static_cast<RenderTarget*>(new DirectX11RenderTarget(*ci));
		}

		DepthTarget* DirectX11GraphicsWrapper::createDepthTarget(DepthTargetCreateInfo ci) {
			return nullptr; //return static_cast<DepthTarget*>(new DirectX11DepthTarget(ci));
		}

		//==================================
		// Deleters
		//==================================
		void DirectX11GraphicsWrapper::deleteRenderTarget(RenderTarget* ptr) {
			delete (DirectX11RenderTarget*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteDepthTarget(DepthTarget* ptr) {
			delete (DirectX11DepthTarget*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteFramebuffer(Framebuffer* ptr) {
			delete (DirectX11Framebuffer*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteVertexBuffer(VertexBuffer* ptr) {
			delete (DirectX11VertexBuffer*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteIndexBuffer(IndexBuffer* ptr) {
			delete (DirectX11IndexBuffer*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteUniformBuffer(UniformBuffer* ptr) {
			delete (DirectX11UniformBuffer*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteUniformBufferBinding(UniformBufferBinding* ptr) {
			delete (DirectX11UniformBufferBinding*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteGraphicsPipeline(GraphicsPipeline* ptr) {
			delete (DirectX11GraphicsPipeline*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteRenderPass(RenderPass* ptr) {
			delete (DirectX11RenderPass*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteTexture(Texture* ptr) {
			delete (DirectX11Texture*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteTextureBinding(TextureBinding* ptr) {
			delete (DirectX11TextureBinding*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteTextureBindingLayout(TextureBindingLayout* ptr) {
			delete (DirectX11TextureBindingLayout*)ptr;
		}
		void DirectX11GraphicsWrapper::deleteCommandBuffer(CommandBuffer* ptr) {
			delete (DirectX11CommandBuffer*)ptr;
		}

		//==================================
		// Booleans
		//==================================
		inline const bool DirectX11GraphicsWrapper::shouldUseImmediateMode() {
			return true;
		}
		inline const bool DirectX11GraphicsWrapper::supportsCommandBuffers() {
			return false;
		}
		inline const bool DirectX11GraphicsWrapper::supportsTesselation() {
			return false;
		}
		inline const bool DirectX11GraphicsWrapper::supportsGeometryShader() {
			return false;
		}
		inline const bool DirectX11GraphicsWrapper::supportsComputeShader() {
			return false;
		}
		inline const bool DirectX11GraphicsWrapper::supportsMultiDrawIndirect() {
			return false;
		}

		//==================================
		// Unused
		//==================================
		void DirectX11GraphicsWrapper::deleteVertexArrayObject(VertexArrayObject* ptr) {
			std::cout << "DirectX11GraphicsWrapper::deleteVertexArrayObject is not used\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
			std::cout << "DirectX11GraphicsWrapper::setViewport is not used.\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			static D3D_PRIMITIVE_TOPOLOGY gt[] = { D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ, //LineLoops,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ, //TriangleFans,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
			D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST };
			device_context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
			
			device_context_->DrawIndexed(indexCount, indexOffsetPtr, baseVertex);
		}
		void DirectX11GraphicsWrapper::drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
			static D3D_PRIMITIVE_TOPOLOGY gt[] = { D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ, //LineLoops,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ, //TriangleFans,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
			D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
			D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST };
			device_context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			device_context_->Draw(count, base);
		}
		void DirectX11GraphicsWrapper::setImmediateBlending(BlendMode) {
			std::cout << "DirectX11GraphicsWrapper::SetImmediateBlending is not used.\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::enableDepth(bool state) {
			std::cout << "DirectX11GraphicsWrapper::enableDepth is not used.\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::setColorMask(ColorMask mask) {
			std::cout << "DirectX11GraphicsWrapper::setColorMask is not used.\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::copyToDepthBuffer(DepthTarget* p) {
			std::cout << "DirectX11GraphicsWrapper::copyToDepthBuffer is not used.\n";
			assert(false);
		}
		void DirectX11GraphicsWrapper::bindDefaultFramebuffer(bool depth) {
			std::cout << "DirectX11GraphicsWrapper::bindDefaultFramebuffer is not used.\n";
			assert(false);
		}

		//==================================
		// DLL Interface
		//==================================
		/*GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
			return new DirectX11GraphicsWrapper(createInfo);
		}

		void deleteGraphics(void * ptr) {
			DirectX11GraphicsWrapper * glptr = (DirectX11GraphicsWrapper *)ptr;
			delete glptr;
		}*/
	}
}