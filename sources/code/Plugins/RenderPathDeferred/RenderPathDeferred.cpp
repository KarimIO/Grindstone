#include "RenderPathDeferred.hpp"
#include <Plugins/Materials/MaterialManager.hpp>
#include <Common/Graphics/Core.hpp>
using namespace Grindstone;

void Rendering::RenderPathDeferred::initialize() {
    Renderer::MaterialManager* manager;
    manager->addQueue("Unlit");
    manager->addQueue("Opaque");
    manager->addQueue("Transparent");
}

void Rendering::RenderPathDeferred::render() {
    Renderer::MaterialManager* manager;

	// Cull Geometry

	// Rendering Geometry
	renderGeometryQueueOpaque((uint32_t)Passes::Unlit);
	renderGeometryQueueOpaque((uint32_t)Passes::Opaque);

	// Cull Lights

	// Rendering Lights

	renderGeometryQueueForward((uint32_t)Passes::Transparency);
}

void Rendering::RenderPathDeferred::renderGeometryQueueOpaque(uint32_t id) {
    Renderer::MaterialManager* manager;
	GraphicsAPI::Core* core;
	
	// Sorting front to back
	// - ...

	auto&render_group = manager->render_queues_[id];
	for (auto&pipeline : render_group.pipelines_) {
		// Bind Pipeline
		core->bindPipeline(pipeline.pipeline_);
		
		for (auto&material : pipeline.materials_) {
			// Bind Textures
			// core->bindTexture(material.texture_binding_);

			// Bind Uniforms
			// core->bindUniformBuffer(material.properties_);

			// Render
			for (auto& mesh : material.meshes_) {
			}
		}
	}
}

void Rendering::RenderPathDeferred::renderGeometryQueueForward(uint32_t id) {
	Renderer::MaterialManager* manager;
	GraphicsAPI::Core* core;

	// Sorting back to front
	// - ...

	auto& render_group = manager->render_queues_[id];
	for (auto& pipeline : render_group.pipelines_) {
		// Bind Pipeline
		core->bindPipeline(pipeline.pipeline_);

		for (auto& material : pipeline.materials_) {
			// Bind Textures
			// core->bindTexture(material.texture_binding_);

			// Bind Uniforms
			// core->bindUniformBuffer(material.properties_);

			// Render
			for (auto& mesh : material.meshes_) {
			}
		}
	}
}

void Rendering::RenderPathDeferred::resizeViewport(uint32_t x, uint32_t y) {

}

Rendering::RenderPathDeferred::~RenderPathDeferred() {
}