#pragma once

struct CompilationOptions {
	enum class Target {
		DirectX = 0,
		Vulkan,
		OpenGL
	};

	Target target;
	bool isDebug = false;
};
