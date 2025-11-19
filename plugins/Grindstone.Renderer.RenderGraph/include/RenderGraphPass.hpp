#pragma once

#include <vector>
#include <functional>

#include <Common/HashedString.hpp>
#include "GpuQueue.hpp"
#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"

namespace Grindstone::Renderer {
	class RenderGraphPass {
	public:
		RenderGraphPass() = default;
		RenderGraphPass(HashedString name, GpuQueue queue) : name(name), queue(queue) {}
		RenderGraphPass(const RenderGraphPass& other) = default;
		RenderGraphPass(RenderGraphPass&& other) noexcept = default;
		RenderGraphPass& operator=(const RenderGraphPass& other) = default;
		RenderGraphPass& operator=(RenderGraphPass&& other) noexcept = default;

		void AddInputImage(HashedString name, AttachmentInfo attachmentInfo);
		void AddInputOutputImage(HashedString inName, HashedString outName, AttachmentInfo attachmentInfo);
		void AddOutputImage(HashedString name, AttachmentInfo);

		void AddInputBuffer(HashedString name, BufferInfo bufferInfo);
		void AddInputOutputBuffer(HashedString inName, HashedString outName, BufferInfo bufferInfo);
		void AddOutputBuffer(HashedString name, BufferInfo bufferInfo);

		void RenderEnabled();
		void RenderDisabled();

		// Set up the pass for rendering in the future
		RenderGraphPass& SetOnSetup(std::function<void* ()> fn);

		// Set up the pass for rendering in the future
		RenderGraphPass& SetOnDestroy(std::function<void(void*)> fn);

		// The real rendering of a pass.
		RenderGraphPass& SetOnRenderEnabled(std::function<void(void*)> fn);

		// For when rendering a system is disabled but resources still need to be cleared, etc.
		RenderGraphPass& SetOnRenderDisabledCallback(std::function<void(void*)> fn);
		HashedString GetName() const;

		struct ContextData {
			// This data will be kept as long as this pass is in use. When the pass is removed from all framegraphs, this data will be deleted (uses dynamic allocator).
			void* permanentData = nullptr;

			// This pass will create an instance of this data for each camera (uses dynamic allocator).
			void* perCameraContext = nullptr;

			// This pass will create an allocate this data every frame and dispose of it at the end of the frame (uses frame/stack allocator).
			void* perFrameContext = nullptr;
		};

	protected:
		// What type of oass is this? Used for scheduling.
		GpuQueue queue;

		// The name of this pass (mostly used for debugging).
		HashedString name;

		// The names of all the resources required by this pass.
		std::vector<HashedString> dependencyResourceNames;

		// The names of all the resources emitted by this pass.
		std::vector<HashedString> emittedResourceNames;

		// Set up the pass for rendering in the future
		std::function<void* ()> OnSetup;

		// Set up the pass for rendering in the future
		std::function<void(void*)> OnDestroy;

		// The real rendering of a pass.
		std::function<void(void*)> OnRenderEnabled;

		// For when rendering a system is disabled but resources still need to be cleared, etc.
		std::function<void(void*)> OnRenderDisabled;
	};
}
