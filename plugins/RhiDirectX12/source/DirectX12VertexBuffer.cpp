#include "DirectX12VertexBuffer.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12Utils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
        DirectX12VertexBuffer::DirectX12VertexBuffer(VertexBufferCreateInfo ci) {
            auto device = DirectX12GraphicsWrapper::get().device_;
            HRESULT hr;

            device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
                D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                                // from the upload heap to this heap
                nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
                IID_PPV_ARGS(&buffer_));

            // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
            vertex_buffer_->SetName(L"Vertex Buffer");

            // create upload heap
            // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
            // We will upload the vertex buffer using this heap to the default heap
            ID3D12Resource* vBufferUploadHeap;
            device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
                D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
                nullptr,
                IID_PPV_ARGS(&vBufferUploadHeap));
            vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

            // store vertex buffer in upload heap
            D3D12_SUBRESOURCE_DATA vertexData = {};
            vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
            vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
            vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

            // we are now creating a command with the command list to copy the data from
            // the upload heap to the default heap
            UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

            // transition the vertex buffer data from copy destination state to vertex buffer state
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

            // Now we execute the command list to upload the initial assets (triangle data)
            commandList->Close();
            ID3D12CommandList* ppCommandLists[] = { commandList };
            commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
            fenceValue[frameIndex]++;
            hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
            if (FAILED(hr))
            {
                Running = false;
            }

            // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
            vertex_buffer_view_.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
            vertex_buffer_view_.StrideInBytes = ci.layout->stride;
            vertex_buffer_view_.SizeInBytes = ci.size;
		}

        DirectX12VertexBuffer::~DirectX12VertexBuffer() {
            vertex_buffer_->Release();
        }
	}
}