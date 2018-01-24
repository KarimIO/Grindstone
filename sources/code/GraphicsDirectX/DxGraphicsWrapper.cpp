#include "DxVertexArrayObject.hpp"
#include "DxGraphicsPipeline.hpp"
#include "DxUniformBuffer.hpp"
#include "DxGraphicsWrapper.hpp"
#include <iostream>
#include <array>
#include <assert.h>
#include "DxTexture.hpp"
#include "DxFramebuffer.hpp"

void DxGraphicsWrapper::Clear() {
	float color[4] = { 0.0, 0.0, 0.0, 1.0 };

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

std::vector <IDXGIAdapter*>EnumerateAdapters(void) {
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	IDXGIFactory* pFactory = NULL;


	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
	{
		return vAdapters;
	}


	for (UINT i = 0;
		pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		vAdapters.push_back(pAdapter);
	}


	if (pFactory)
		pFactory->Release();

	return vAdapters;

}

DxGraphicsWrapper::DxGraphicsWrapper(InstanceCreateInfo createInfo) {
	title = createInfo.title;
	vsync = createInfo.vsync;
	width = createInfo.width;
	height = createInfo.height;
	debug = createInfo.debug;
	input = createInfo.inputInterface;

	InitializeWin32Window();

	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;
	char m_videoCardDescription[128];

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i<numModes; i++) {
		if (displayModeList[i].Width == (unsigned int)createInfo.width) {
			if (displayModeList[i].Height == (unsigned int)createInfo.height) {
				numerator = displayModeList[i].RefreshRate.Numerator;
denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)) {
		return;
	}

	// Store the dedicated video card memory in megabytes.
	uint32_t m_videoCardMemory = (uint32_t)(adapterDesc.DedicatedVideoMemory);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s((size_t *)&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0) {
		return;
	}

	std::cout << "Using " << m_videoCardDescription << "\n";

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (vsync) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = window_handle;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = true;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	if (debug)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1,
									D3D_FEATURE_LEVEL_11_0,
									D3D_FEATURE_LEVEL_10_1,
									D3D_FEATURE_LEVEL_10_0 };

	// Create the swap chain, Direct3D device, and Direct3D device context.
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels, sizeof(featureLevels) / sizeof(featureLevels[0]),
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext))) {
		std::cerr << "Failed to create a DirectX device, or no DirectX 10+ capable system detected.\n";
		return;
	}

	switch (m_device->GetFeatureLevel()) {
	case D3D_FEATURE_LEVEL_11_1:
		std::cout << "DirectX 11.1 device created\n";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		std::cout << "DirectX 11.0 device created\n";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		std::cout << "DirectX 10.1 device created\n";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		std::cout << "DirectX 10.0 device created\n";
		break;
	}

	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) {
		return;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result)) {
		return;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result)) {
		return;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result)) {
		return;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result)) {
		return;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

	D3D11_BLEND_DESC BlendStateDescription;
	ZeroMemory(&BlendStateDescription, sizeof(D3D11_BLEND_DESC));

	BlendStateDescription.AlphaToCoverageEnable = FALSE;
	BlendStateDescription.IndependentBlendEnable = FALSE;
	BlendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	BlendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	BlendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	BlendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	BlendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	result = m_device->CreateBlendState(&BlendStateDescription, &m_addBlendState);
	if (FAILED(result)) {
		return;
	}

	BlendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	BlendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	result = m_device->CreateBlendState(&BlendStateDescription, &m_alphaBlendState);
	if (FAILED(result)) {
		return;
	}

	BlendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	result = m_device->CreateBlendState(&BlendStateDescription, &m_noBlendState);
	if (FAILED(result)) {
		return;
	}

	// Setup the viewport for rendering.
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);
	std::cout << "Successful Initialization of DirectX\n============================\n";
}

void DxGraphicsWrapper::CreateDefaultStructures() {
}

void DxGraphicsWrapper::CreateDefaultFramebuffers(DefaultFramebufferCreateInfo createInfo, Framebuffer **&framebuffers, uint32_t &framebufferCount) {
	framebufferCount = 0;
}

uint32_t DxGraphicsWrapper::GetImageIndex() {
	return 0;
}

CommandBuffer *DxGraphicsWrapper::CreateCommandBuffer(CommandBufferCreateInfo ci) {
	return nullptr;
}

VertexArrayObject * DxGraphicsWrapper::CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) {
	return (VertexArrayObject *)new DxVertexArrayObject(ci);
}

VertexBuffer *DxGraphicsWrapper::CreateVertexBuffer(VertexBufferCreateInfo ci) {
	return (VertexBuffer *)new DxVertexBuffer(m_device, m_deviceContext, ci);
}

IndexBuffer *DxGraphicsWrapper::CreateIndexBuffer(IndexBufferCreateInfo ci) {
	return (IndexBuffer *)new DxIndexBuffer(m_device, m_deviceContext, ci);
}

UniformBuffer * DxGraphicsWrapper::CreateUniformBuffer(UniformBufferCreateInfo ci) {
	return (UniformBuffer *)new DxUniformBuffer(m_device, m_deviceContext, ci);
}

UniformBufferBinding *DxGraphicsWrapper::CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
	return (UniformBufferBinding *)new DxUniformBufferBinding(ci);
}

Texture * DxGraphicsWrapper::CreateCubemap(CubemapCreateInfo createInfo) {
	return (Texture *)new DxTexture(m_device, m_deviceContext, createInfo);
}

Texture * DxGraphicsWrapper::CreateTexture(TextureCreateInfo createInfo) {
	return (Texture *)new DxTexture(m_device, m_deviceContext, createInfo);
}

TextureBinding * DxGraphicsWrapper::CreateTextureBinding(TextureBindingCreateInfo createInfo) {
	return (TextureBinding *)new DxTextureBinding(m_deviceContext, createInfo);
}

TextureBindingLayout * DxGraphicsWrapper::CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) {
	return (TextureBindingLayout *)new DxTextureBindingLayout(createInfo);
}

Framebuffer * DxGraphicsWrapper::CreateFramebuffer(FramebufferCreateInfo ci) {
	return (Framebuffer *)new DxFramebuffer(m_device, m_deviceContext, ci);
}

RenderTarget *DxGraphicsWrapper::CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc) {
	return static_cast<RenderTarget *>(new DxRenderTarget(m_device, m_deviceContext, rt, rc));
}

DepthTarget *DxGraphicsWrapper::CreateDepthTarget(DepthTargetCreateInfo rt) {
	return static_cast<DepthTarget *>(new DxDepthTarget(m_device, m_deviceContext, rt));
}

void DxGraphicsWrapper::CopyToDepthBuffer(DepthTarget * p) {
	DxDepthTarget *dxd = static_cast<DxDepthTarget *>(p);
	m_deviceContext->CopyResource(m_depthStencilBuffer, dxd->getTexture());
}

RenderPass *DxGraphicsWrapper::CreateRenderPass(RenderPassCreateInfo ci) {
	return nullptr;
}

GraphicsPipeline * DxGraphicsWrapper::CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
	return (GraphicsPipeline *)new DxGraphicsPipeline(m_device, m_deviceContext, ci);
}

bool DxGraphicsWrapper::SupportsCommandBuffers() {
	return false;
}

bool DxGraphicsWrapper::SupportsTesselation() {
	return false;
}

bool DxGraphicsWrapper::SupportsGeometryShader() {
	return false;
}

bool DxGraphicsWrapper::SupportsComputeShader() {
	return true;
}

bool DxGraphicsWrapper::SupportsMultiDrawIndirect() {
	return false;
}

void DxGraphicsWrapper::BindTextureBinding(TextureBinding *tb) {
	DxTextureBinding *dtb = (DxTextureBinding *)tb;
	dtb->Bind();
}

void DxGraphicsWrapper::BindVertexArrayObject(VertexArrayObject *vao) {
	DxVertexArrayObject *nvao = (DxVertexArrayObject *)vao;
	nvao->Bind();
}

void DxGraphicsWrapper::WaitUntilIdle() {
}

void DxGraphicsWrapper::DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) {
}

void DxGraphicsWrapper::DrawImmediateIndexed(bool patches, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	m_deviceContext->DrawIndexed(indexCount, indexOffsetPtr, baseVertex);
}

void DxGraphicsWrapper::DrawImmediateVertices(uint32_t base, uint32_t count) {
	m_deviceContext->Draw(count, base);
}

void DxGraphicsWrapper::SetImmediateBlending(BlendMode state) {
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	switch (state) {
	case BLEND_NONE:
		m_deviceContext->OMSetBlendState(m_noBlendState, blendFactor, 0xffffffff);
		break;
	case BLEND_ADDITIVE:
		m_deviceContext->OMSetBlendState(m_addBlendState, blendFactor, 0xffffffff);
		break;
	case BLEND_ADD_ALPHA:
		m_deviceContext->OMSetBlendState(m_alphaBlendState, blendFactor, 0xffffffff);
		break;
	}
}

void DxGraphicsWrapper::BindDefaultFramebuffer(bool depth) {
	if (depth) {
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	}
	else {
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
	}
}

void DxGraphicsWrapper::EnableDepth(bool state) {
}

ColorFormat DxGraphicsWrapper::GetDeviceColorFormat() {
	return ColorFormat();
}

void DxGraphicsWrapper::SwapBuffer() {
	m_swapChain->Present((UINT)vsync, 0);
}

void DxGraphicsWrapper::Cleanup() {
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain) {
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState) {
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView) {
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState) {
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer) {
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView) {
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext) {
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device) {
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain) {
		m_swapChain->Release();
		m_swapChain = 0;
	}
}

void DxGraphicsWrapper::DeleteFramebuffer(Framebuffer *ptr) {
}

void DxGraphicsWrapper::DeleteVertexBuffer(VertexBuffer *ptr) {
	delete (DxVertexBuffer *)ptr;
}

void DxGraphicsWrapper::DeleteIndexBuffer(IndexBuffer *ptr) {
	delete (DxIndexBuffer *)ptr;
}

void DxGraphicsWrapper::DeleteUniformBuffer(UniformBuffer *ptr) {
	delete (DxUniformBuffer *)ptr;
}

void DxGraphicsWrapper::DeleteUniformBufferBinding(UniformBufferBinding * ptr) {
	delete (DxUniformBufferBinding *)ptr;
}

void DxGraphicsWrapper::DeleteGraphicsPipeline(GraphicsPipeline *ptr) {
	delete (DxGraphicsPipeline *)ptr;
}

void DxGraphicsWrapper::DeleteRenderPass(RenderPass *ptr) {
}

void DxGraphicsWrapper::DeleteTexture(Texture * ptr) {
}

void DxGraphicsWrapper::DeleteCommandBuffer(CommandBuffer *ptr) {
}

void DxGraphicsWrapper::DeleteVertexArrayObject(VertexArrayObject * ptr) {
	delete (DxVertexArrayObject *)ptr;
}

GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
	return new DxGraphicsWrapper(createInfo);
}

GRAPHICS_EXPORT void deleteGraphics(void * ptr) {
	free(ptr);
}