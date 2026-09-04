#pragma once

#include <vector>

#include <DebugDraw.h>
#include <RecastDebugDraw.h>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::Ai {
	struct NavMeshDebugRenderer : public duDebugDraw {
		struct DebugVertex {
			float x, y, z;
			float u, v;
			uint32_t color;
		};

		struct DrawCall {
			duDebugDrawPrimitives primitiveType;
			float                 pointOrLineSize;
			bool                  depthTest;
			bool                  textured;
			uint32_t              vertexOffset;
			uint32_t              vertexCount;
		};

		void Initialize();
		static NavMeshDebugRenderer* GetInstance();
		
		void Clear();
		void BuildVertexBuffers();

		bool ShouldRenderThisFrame() const;
		void DrawDebug(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, Grindstone::GraphicsPipelineAsset* pipelineAsset);

		virtual void depthMask(bool state) override;

		virtual void texture(bool state) override;

		/// Begin drawing primitives.
		///  @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
		///  @param size [in] size of a primitive, applies to point size and line width only.
		virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f) override;

		/// Submit a vertex
		///  @param pos [in] position of the verts.
		///  @param color [in] color of the verts.
		virtual void vertex(const float* pos, unsigned int color) override;

		/// Submit a vertex
		///  @param x,y,z [in] position of the verts.
		///  @param color [in] color of the verts.
		virtual void vertex(const float x, const float y, const float z, unsigned int color) override;

		/// Submit a vertex
		///  @param pos [in] position of the verts.
		///  @param color [in] color of the verts.
		///  @param uv [in] the uv coordinates of the verts.
		virtual void vertex(const float* pos, unsigned int color, const float* uv) override;

		/// Submit a vertex
		///  @param x,y,z [in] position of the verts.
		///  @param color [in] color of the verts.
		///  @param u,v [in] the uv coordinates of the verts.
		virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) override;

		/// End drawing primitives.
		virtual void end() override;

		/// Compute a color for given area.
		virtual unsigned int areaToCol(unsigned int area) override;

	protected:
		std::vector<DebugVertex>		vertices;
		std::vector<DrawCall>           drawCalls;
		DrawCall*                       currentCall = nullptr;
		bool                            isDepthTestEnabled = true;
		bool                            isTextured = false;
		Grindstone::GraphicsAPI::Buffer* vertexBuffer = nullptr;
		Grindstone::GraphicsAPI::VertexInputLayout vertexLayout;
	};
}
