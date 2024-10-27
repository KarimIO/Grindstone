#include "../pch.h"

#define NOMINMAX

#include <cassert>

#include <windows.h>
#include <windowsx.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12CommandBuffer.hpp"
#include "DirectX12DepthStencilTarget.hpp"
#include "DirectX12Descriptors.hpp"
#include "DirectX12Framebuffer.hpp"
#include "DirectX12GraphicsPipeline.hpp"
#include "DirectX12IndexBuffer.hpp"
#include "DirectX12RenderPass.hpp"
#include "DirectX12RenderTarget.hpp"
#include "DirectX12Texture.hpp"
#include "DirectX12UniformBuffer.hpp"
#include "DirectX12VertexBuffer.hpp"
#include "DirectX12Format.hpp"
#include <set>
#include <algorithm>
#include <array>

#include "Window/Win32Window.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12GraphicsWrapper *DirectX12GraphicsWrapper::graphics_wrapper_ = nullptr;

		bool DirectX12GraphicsWrapper::initialize(GraphicsWrapperCreateInfo ci) {
			api_type_ = GraphicsAPIType::DirectX12;
			graphics_wrapper_ = this;
			debug_ = ci.debug;
			window_ = ci.window;

			if (ci.debug)
				setupDebugMessenger();
			pickPhysicalDevice();
			createLogicalDevice();
			createCommandQueues();
			createSwapChain();
			createCommandPool();
			createSyncObjects();
			createDescriptorPool();

			return true;
		}

		void DirectX12GraphicsWrapper::setupDebugMessenger() {
#if defined(_DEBUG)
			// Always enable the debug layer before doing anything DX12 related
			// so all possible errors generated while creating DX12 objects
			// are caught by the debug layer.
			ComPtr<ID3D12Debug> debugInterface;
			if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)) < 0)
				throw std::runtime_error("DirectX Debugger failed");

			debugInterface->EnableDebugLayer();
#endif
		}

		void DirectX12GraphicsWrapper::pickPhysicalDevice() {
			ComPtr<IDXGIFactory4> dxgiFactory;
			UINT createFactoryFlags = 0;
#if defined(_DEBUG)
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

			if (CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) < 0)
				throw std::runtime_error("DirectX: Could not create dxgiFactory!");

			ComPtr<IDXGIAdapter1> dxgiAdapter1;
			ComPtr<IDXGIAdapter4> dxgiAdapter4;

			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				// Check to see if the adapter can create a D3D12 device without actually
				// creating it. The adapter with the largest dedicated video memory
				// is favored.
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					if (dxgiAdapter1.As(&dxgiAdapter4) < 0)
						throw std::runtime_error("Could not convert to dxgiAdapter4!");
				}
			}

			api_version_ = "12.0";
			DXGI_ADAPTER_DESC adapterDescription;
			dxgiAdapter4.Get()->GetDesc(&adapterDescription);
			std::wstring dc = adapterDescription.Description;
			const char* vendor_name = getVendorNameFromID(adapterDescription.VendorId);
			if (vendor_name == 0) {
				vendor_name_ = std::string("Unknown Vendor(") + std::to_string(adapterDescription.VendorId) + ")";
			}
			else {
				vendor_name_ = vendor_name;
			}
			adapter_name_ = std::string(dc.begin(), dc.end());

			dxgi_adapter_ = dxgiAdapter4;
		}

		void DirectX12GraphicsWrapper::createLogicalDevice() {
			ComPtr<ID3D12Device2> d3d12Device2;
			if (D3D12CreateDevice(dxgi_adapter_.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)) < 0)
				throw std::runtime_error("DirectX: Could not create device!");

			// Enable debug messages in debug mode.
#if defined(_DEBUG)
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
			{
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

				// Suppress whole categories of messages
				//D3D12_MESSAGE_CATEGORY Categories[] = {};

				// Suppress messages based on their severity level
				D3D12_MESSAGE_SEVERITY Severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};

				// Suppress individual messages by their ID
				D3D12_MESSAGE_ID DenyIds[] = {
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
				};

				D3D12_INFO_QUEUE_FILTER NewFilter = {};
				//NewFilter.DenyList.NumCategories = _countof(Categories);
				//NewFilter.DenyList.pCategoryList = Categories;
				NewFilter.DenyList.NumSeverities = _countof(Severities);
				NewFilter.DenyList.pSeverityList = Severities;
				NewFilter.DenyList.NumIDs = _countof(DenyIds);
				NewFilter.DenyList.pIDList = DenyIds;

				if (pInfoQueue->PushStorageFilter(&NewFilter) < 0)
					throw std::runtime_error("DirectX: Could not PushStorageFilter!");
			}
#endif

			device_ = d3d12Device2;
		}

		void DirectX12GraphicsWrapper::createSwapChain() {
			ComPtr<IDXGISwapChain4> dxgiSwapChain4;
			ComPtr<IDXGIFactory4> dxgiFactory4;
			UINT createFactoryFlags = 0;
#if defined(_DEBUG)
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

			if (CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)) < 0) {
				throw std::runtime_error("DirectX12: Failed to CreateDXGIFactory!");
			}

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Width = 0; // width;
			swapChainDesc.Height = 0; // height;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.Stereo = FALSE;
			swapChainDesc.SampleDesc = { 1, 0 };
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = num_buffers; // bufferCount;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			// It is recommended to always allow tearing if tearing support is available.
			swapChainDesc.Flags = 0; // CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

			auto hWnd = ((Win32Window*)window_)->getHandle();
			ComPtr<IDXGISwapChain1> swapChain1;
			if (dxgiFactory4->CreateSwapChainForHwnd(
				command_queue_.Get(),
				hWnd,
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain1) < 0)
				throw std::runtime_error("DirectX12: Failed to CreateDXGIFactory!");

			// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
			// will be handled manually.
			if(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER) < 0)
				throw std::runtime_error("DirectX12: Failed to MakeWindowAssociation!");

			if(swapChain1.As(&dxgiSwapChain4) < 0)
				throw std::runtime_error("DirectX12: Failed to convert dxgiSwapChain4 to swapChain1!");

			swap_chain_ = dxgiSwapChain4;
			current_backbuffer_index_ = swap_chain_->GetCurrentBackBufferIndex();

			// Render Target
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors = num_buffers;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

			if (device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rt_descriptor_heap_)) < 0)
				throw std::runtime_error("DirectX12: Failed to CreateDescriptorHeap!");
#if 0
			rt_descriptor_size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			for (int i = 0; i < num_buffers; i++) {
				// first we get the n'th buffer in the swap chain and store it in the n'th
				// position of our ID3D12Resource array
				if (swap_chain_->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])) < 0) {
					throw std::runtime_error("DirectX12: Failed to GetBuffer!");
				}

				// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
				device_->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

				// we increment the rtv handle by the rtv descriptor size we got above
				rtvHandle.Offset(1, rtvDescriptorSize);
			}
#endif
		}

		void DirectX12GraphicsWrapper::createCommandQueues() {
			ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 0;

			if (device_->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)) < 0)
				throw std::runtime_error("DirectX12: Failed to CreateDXGIFactory!");

			command_queue_ = d3d12CommandQueue;
		}

		DirectX12GraphicsWrapper::~DirectX12GraphicsWrapper() {

		}

		void DirectX12GraphicsWrapper::createCommandPool() {
			if (device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&graphics_command_allocator_)) < 0)
				throw std::runtime_error("DirectX12: Failed to CreateCommandAllocator!");
		}

		uint32_t DirectX12GraphicsWrapper::getImageIndex()
		{
			return uint32_t();
		}

		void DirectX12GraphicsWrapper::waitUntilIdle() {
#if 0
			// swap the current rtv buffer index so we draw on the correct buffer
			frameIndex = swap_chain_->GetCurrentBackBufferIndex();

			// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
			// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
			if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex]) {
				// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
				if (fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent) < 0) {
					Running = false;
				}

				// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
				// has reached "fenceValue", we know the command queue has finished executing
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			// increment fenceValue for next frame
			fenceValue[frameIndex]++;
#endif
		}

		void DirectX12GraphicsWrapper::createSyncObjects() {
			for (int i = 0; i < num_buffers; i++) {
				HRESULT hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_[i]));
				if (FAILED(hr)) {
					return;
				}

				fence_value_[i] = 0; // set the initial fence value to 0
			}

			// create a handle to a fence event
			fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (fence_event_ == nullptr) {
				return;
			}

		}

		void DirectX12GraphicsWrapper::createDescriptorPool() {
		}

		void DirectX12GraphicsWrapper::drawCommandBuffers(uint32_t ii, CommandBuffer** command_buffers, uint32_t command_buffer_count) {
			ID3D12CommandList** command_lists = new ID3D12CommandList * [command_buffer_count];
			for (int i = 0; i < command_buffer_count; ++i)
				((DirectX12CommandBuffer*)command_buffers)->getCommandList();

			// execute the array of command lists
			command_queue_->ExecuteCommandLists(command_buffer_count, command_lists);

			// this command goes in at the end of our command queue. we will know when our command queue
			// has finished because the fence value will be set to "fenceValue" from the GPU since the command
			// queue is being executed on the GPU
			HRESULT hr = command_queue_->Signal(fence_[frame_index_], fence_value_[frame_index_]);
			if (FAILED(hr)) {
				throw std::runtime_error("DirectX12: Signal failed");
			}
		}

		DirectX12GraphicsWrapper& DirectX12GraphicsWrapper::get() {
			return *graphics_wrapper_;
		}

		void DirectX12GraphicsWrapper::adjustPerspective(float* perspective) {
			// TODO: Fix
			// perspective[1 * 4 + 1] *= -1;
		}

		void DirectX12GraphicsWrapper::registerWindow(Window* window) {
			//window->addBinding(new DirectX12WindowGraphicsBinding());
		}

		//==================================
		// Get Text Metainfo
		//==================================
		const char* DirectX12GraphicsWrapper::getVendorName() {
			return vendor_name_.c_str();
		}

		const char* DirectX12GraphicsWrapper::getAdapterName() {
			return adapter_name_.c_str();
		}

		const char* DirectX12GraphicsWrapper::getAPIName() {
			return "DirectX12";
		}

		const char* DirectX12GraphicsWrapper::getAPIVersion() {
			return api_version_.c_str();
		}

		//==================================
		// Creators
		//==================================
		Framebuffer* DirectX12GraphicsWrapper::createFramebuffer(FramebufferCreateInfo ci) {
			return nullptr; // return static_cast<Framebuffer*>(new DirectX12Framebuffer(ci));
		}

		RenderPass* DirectX12GraphicsWrapper::createRenderPass(RenderPassCreateInfo ci) {
			return nullptr; //return static_cast<RenderPass*>(new DirectX12RenderPass(ci));
		}

		GraphicsPipeline* DirectX12GraphicsWrapper::createGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			return static_cast<GraphicsPipeline*>(new DirectX12GraphicsPipeline(ci));
		}

		CommandBuffer* DirectX12GraphicsWrapper::createCommandBuffer(CommandBufferCreateInfo ci) {
			return static_cast<CommandBuffer*>(new DirectX12CommandBuffer(ci));
		}

		VertexBuffer* DirectX12GraphicsWrapper::createVertexBuffer(VertexBufferCreateInfo ci) {
			return nullptr; //return static_cast<VertexBuffer*>(new DirectX12VertexBuffer(ci));
		}

		IndexBuffer* DirectX12GraphicsWrapper::createIndexBuffer(IndexBufferCreateInfo ci) {
			return nullptr; //return static_cast<IndexBuffer*>(new DirectX12IndexBuffer(ci));
		}

		UniformBuffer* DirectX12GraphicsWrapper::createUniformBuffer(UniformBufferCreateInfo ci) {
			return nullptr; //return static_cast<UniformBuffer*>(new DirectX12UniformBuffer(ci));
		}

		UniformBufferBinding* DirectX12GraphicsWrapper::createUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			return nullptr; //return static_cast<UniformBufferBinding*>(new DirectX12UniformBufferBinding(ci));
		}

		Texture* DirectX12GraphicsWrapper::createCubemap(CubemapCreateInfo ci) {
			return nullptr; // static_cast<Texture *>(new DirectX12Texture(ci));
		}

		Texture* DirectX12GraphicsWrapper::createTexture(TextureCreateInfo ci) {
			return nullptr; //return static_cast<Texture*>(new DirectX12Texture(ci));
		}

		TextureBinding* DirectX12GraphicsWrapper::createTextureBinding(TextureBindingCreateInfo ci) {
			return nullptr; //return static_cast<TextureBinding*>(new DirectX12TextureBinding(ci));
		}

		TextureBindingLayout* DirectX12GraphicsWrapper::createTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			return nullptr; //return static_cast<TextureBindingLayout*>(new DirectX12TextureBindingLayout(ci));
		}

		RenderTarget* DirectX12GraphicsWrapper::createRenderTarget(RenderTargetCreateInfo* ci, uint32_t rc, bool cube) {
			return nullptr; //return static_cast<RenderTarget*>(new DirectX12RenderTarget(*ci));
		}

		DepthStencilTarget* DirectX12GraphicsWrapper::createDepthStencilTarget(DepthStencilTargetCreateInfo ci) {
			return nullptr; //return static_cast<DepthStencilTarget*>(new DirectX12DepthStencilTarget(ci));
		}

		//==================================
		// Deleters
		//==================================
		void DirectX12GraphicsWrapper::deleteRenderTarget(RenderTarget* ptr) {
			delete (DirectX12RenderTarget*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteDepthStencilTarget(DepthStencilTarget* ptr) {
			delete (DirectX12DepthStencilTarget*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteFramebuffer(Framebuffer* ptr) {
			delete (DirectX12Framebuffer*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteVertexBuffer(VertexBuffer* ptr) {
			delete (DirectX12VertexBuffer*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteIndexBuffer(IndexBuffer* ptr) {
			delete (DirectX12IndexBuffer*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteUniformBuffer(UniformBuffer* ptr) {
			delete (DirectX12UniformBuffer*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteUniformBufferBinding(UniformBufferBinding* ptr) {
			delete (DirectX12UniformBufferBinding*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteGraphicsPipeline(GraphicsPipeline* ptr) {
			delete (DirectX12GraphicsPipeline*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteRenderPass(RenderPass* ptr) {
			delete (DirectX12RenderPass*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteTexture(Texture* ptr) {
			delete (DirectX12Texture*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteTextureBinding(TextureBinding* ptr) {
			delete (DirectX12TextureBinding*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteTextureBindingLayout(TextureBindingLayout* ptr) {
			delete (DirectX12TextureBindingLayout*)ptr;
		}
		void DirectX12GraphicsWrapper::deleteCommandBuffer(CommandBuffer* ptr) {
			delete (DirectX12CommandBuffer*)ptr;
		}

		//==================================
		// Booleans
		//==================================
		inline const bool DirectX12GraphicsWrapper::shouldUseImmediateMode() {
			return false;
		}
		inline const bool DirectX12GraphicsWrapper::supportsCommandBuffers() {
			return false;
		}
		inline const bool DirectX12GraphicsWrapper::supportsTesselation() {
			return false;
		}
		inline const bool DirectX12GraphicsWrapper::supportsGeometryShader() {
			return false;
		}
		inline const bool DirectX12GraphicsWrapper::supportsComputeShader() {
			return false;
		}
		inline const bool DirectX12GraphicsWrapper::supportsMultiDrawIndirect() {
			return false;
		}

		//==================================
		// Unused
		//==================================
		VertexArrayObject* DirectX12GraphicsWrapper::createVertexArrayObject(VertexArrayObjectCreateInfo ci) {
			std::cout << "DirectX12GraphicsWrapper::createVertexArrayObject is not used.\n";
			assert(false);
			return nullptr;
		}
		void DirectX12GraphicsWrapper::deleteVertexArrayObject(VertexArrayObject* ptr) {
			std::cout << "DirectX12GraphicsWrapper::deleteVertexArrayObject is not used\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
			std::cout << "DirectX12GraphicsWrapper::clear is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::bindTextureBinding(TextureBinding*) {
			std::cout << "DirectX12GraphicsWrapper::bindTextureBinding is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::bindVertexArrayObject(VertexArrayObject*) {
			std::cout << "DirectX12GraphicsWrapper::bindVertexArrayObject is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			std::cout << "DirectX12GraphicsWrapper::drawImmediateIndexed is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
			std::cout << "DirectX12GraphicsWrapper::drawImmediateVertices is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::setImmediateBlending(BlendMode) {
			std::cout << "DirectX12GraphicsWrapper::SetImmediateBlending is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::enableDepth(bool state) {
			std::cout << "DirectX12GraphicsWrapper::enableDepth is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::setColorMask(ColorMask mask) {
			std::cout << "DirectX12GraphicsWrapper::setColorMask is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::copyToDepthBuffer(DepthStencilTarget* p) {
			std::cout << "DirectX12GraphicsWrapper::copyToDepthBuffer is not used.\n";
			assert(false);
		}
		void DirectX12GraphicsWrapper::bindDefaultFramebuffer(bool depth) {
			std::cout << "DirectX12GraphicsWrapper::bindDefaultFramebuffer is not used.\n";
			assert(false);
		}

		//==================================
		// DLL Interface
		//==================================
		/*GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
			return new DirectX12GraphicsWrapper(createInfo);
		}

		void deleteGraphics(void * ptr) {
			DirectX12GraphicsWrapper * glptr = (DirectX12GraphicsWrapper *)ptr;
			delete glptr;
		}*/
	}
}
