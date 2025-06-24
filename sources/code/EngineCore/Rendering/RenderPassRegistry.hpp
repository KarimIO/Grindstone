#pragma once

#include <unordered_map>

#include <Common/HashedString.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
	};

	class RenderPassRegistry {
	public:
		virtual void RegisterRenderpass(Grindstone::HashedString hashedString, Grindstone::GraphicsAPI::RenderPass* rp);
		virtual Grindstone::GraphicsAPI::RenderPass* GetRenderpass(Grindstone::HashedString hashedString);
		virtual void UnregisterRenderpass(Grindstone::HashedString hashedString);

	private:
		std::unordered_map<Grindstone::HashedString, Grindstone::GraphicsAPI::RenderPass*> registry;
	};
}
