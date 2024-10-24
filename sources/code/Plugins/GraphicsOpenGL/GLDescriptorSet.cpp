#include <GL/gl3w.h>
#include "GLDescriptorSet.hpp"
#include <iostream>

using namespace Grindstone::GraphicsAPI;

GLDescriptorSet::GLDescriptorSet(CreateInfo& createInfo) {
	ChangeBindings(createInfo.bindings, createInfo.bindingCount);
}

GLDescriptorSet::~GLDescriptorSet() {
}

void GLDescriptorSet::ChangeBindings(Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset) {
}
