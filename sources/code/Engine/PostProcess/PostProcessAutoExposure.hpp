#ifndef _POST_PROCESS_AUTO_EXPOSURE_HPP
#define _POST_PROCESS_AUTO_EXPOSURE_HPP

#include "BasePost.hpp"

namespace Grindstone {
	namespace GraphicsPipeline {
		class GraphicsPipeline;
		class UniformBuffer;
	}
}

struct RenderTargetContainer;

class PostProcessAutoExposure : public BasePostProcess {
public:
	PostProcessAutoExposure(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target);
	virtual ~PostProcessAutoExposure() override;
    virtual void Process();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
	float GetExposure();
private:
	RenderTargetContainer *source_;
	RenderTargetContainer *target_;
	Grindstone::GraphicsAPI::TextureSubBinding *tonemap_sub_binding_;
	Grindstone::GraphicsAPI::TextureBindingLayout *tonemap_tbl_;

	Grindstone::GraphicsAPI::GraphicsPipeline *gpipeline_;

	Grindstone::GraphicsAPI::RenderTarget *lum_buffer_;
	Grindstone::GraphicsAPI::Framebuffer *lum_framebuffer_;
};

#endif // !POST_PROCESS_AUTO_EXPOSURE_HPP