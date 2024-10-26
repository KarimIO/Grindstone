#include <iostream>

#include <GL/gl3w.h>

#include "GLDescriptorSet.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::DescriptorSet::DescriptorSet(const DescriptorSet::CreateInfo& createInfo) {
	ChangeBindings(createInfo.bindings, createInfo.bindingCount);
}

OpenGL::DescriptorSet::~DescriptorSet() {
}

void OpenGL::DescriptorSet::ChangeBindings(Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset) {
}
