#pragma once

#include <stdint.h>

#include <Common/Graphics/DepthStencilTarget.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class DepthStencilTarget : public Grindstone::GraphicsAPI::DepthStencilTarget {
	public:
		DepthStencilTarget(const CreateInfo& cis);
		uint32_t GetHandle() const;

		bool IsCubemap() const;

		virtual void Resize(uint32_t width, uint32_t height);
		virtual void BindFace(int k);
		virtual void Bind(int i);
		virtual ~DepthStencilTarget();
	private:
		void CreateDepthStencilTarget();
	private:
		DepthFormat depthFormat;
		uint32_t width;
		uint32_t height;

		uint32_t handle;
		bool isShadowMap;
		bool isCubemap;
	};
}
