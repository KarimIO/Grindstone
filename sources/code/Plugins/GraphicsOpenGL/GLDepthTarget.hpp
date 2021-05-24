#ifndef _GL_DEPTH_TARGET_H
#define _GL_DEPTH_TARGET_H

#include <stdint.h>
#include <Common/Graphics/DepthTarget.hpp>


namespace Grindstone {
	namespace GraphicsAPI {
		class GLDepthTarget : public DepthTarget {
		public:
			GLDepthTarget(CreateInfo& cis);
			uint32_t GetHandle();

			bool IsCubemap();

			virtual void Resize(uint32_t width, uint32_t height);
			virtual void BindFace(int k);
			virtual void Bind(int i);
			virtual ~GLDepthTarget();
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
}

#endif