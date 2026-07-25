#pragma once

#include <string>
#include <vector>

#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "../Assets/Mesh3dAsset.hpp"
#include "../Components/AnimatorComponent.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Buffer;
		class DescriptorSet;
		class VertexArrayObject;
	}

	struct SkeletalMeshComponent {
		AssetReference<Mesh3dAsset> mesh;
		Grindstone::GraphicsAPI::Buffer* vertexPositionBuffer = nullptr;
		Grindstone::GraphicsAPI::Buffer* vertexNormalBuffer = nullptr;
		Grindstone::GraphicsAPI::Buffer* vertexTangentBuffer = nullptr;
		Grindstone::GraphicsAPI::Buffer* vertexCountBuffer = nullptr;
		Grindstone::GraphicsAPI::VertexArrayObject* skinnedVertexArrayObject = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* skinningDescriptorSet = nullptr;

		REFLECT("SkeletalMesh")
	};
}
