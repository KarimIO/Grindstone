#pragma once

#include <stdint.h>

#include <Common/Graphics/DepthTarget.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class DepthTarget : public Grindstone::GraphicsAPI::DepthTarget {
	public:
		DepthTarget(const CreateInfo& cis);
		uint32_t GetHandle() const;

		bool IsCubemap() const;

		virtual void Resize(uint32_t width, uint32_t height);
		virtual void BindFace(int k);
		virtual void Bind(int i);
		virtual ~DepthTarget();
	private:
		void CreateDepthTarget();
	private:
		DepthFormat depthFormat;
		uint32_t width;
		uint32_t height;

		uint32_t handle;
		bool isShadowMap;
		bool isCubemap;
	};
}
