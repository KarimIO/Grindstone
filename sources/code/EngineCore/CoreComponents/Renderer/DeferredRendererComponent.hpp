#pragma once

#include "BaseRendererComponent.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Graphics/Framebuffer.hpp"

namespace Grindstone {
	struct DeferredRendererComponent : public BaseRendererComponent {
	private:
		GraphicsAPI::Framebuffer* framebuffer;

		REFLECT("Deferred Renderer")
	};
}
