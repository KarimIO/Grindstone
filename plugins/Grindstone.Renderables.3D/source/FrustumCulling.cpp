#include <glm/gtx/transform.hpp>
#include <Grindstone.Renderables.3D/include/FrustumCulling.hpp>

using namespace Grindstone;

// Thanks to Bruno Opsenica https://bruop.github.io/improved_frustum_culling/
bool Grindstone::Renderer::IsInFrustum(const CullingFrustum& frustum, const glm::mat4& viewModelMatrix, const AABB& aabb) {
	// TODO: Fix this
	if (frustum.isOrtho) {
		return true;
	}

	constexpr size_t cornerCount = 4;

	float z_near = frustum.nearDistance;
	float z_far = frustum.farDistance;

	float x_near = frustum.nearRight;
	float y_near = frustum.nearTop;

	// Consider four adjacent corners of the ABB
	glm::vec3 corners[] = {
		{aabb.min.x, aabb.min.y, aabb.min.z},
		{aabb.max.x, aabb.min.y, aabb.min.z},
		{aabb.min.x, aabb.max.y, aabb.min.z},
		{aabb.min.x, aabb.min.y, aabb.max.z},
	};

	// NOTE: Only works with affine matrices
	for (size_t corner_idx = 0; corner_idx < cornerCount; corner_idx++) {
		corners[corner_idx] = (viewModelMatrix * glm::vec4(corners[corner_idx], 1.0f));
	}

	OBB obb = {
		.axes = {
			corners[1] - corners[0],
			corners[2] - corners[0],
			corners[3] - corners[0]
		},
	};

	obb.center = corners[0] + 0.5f * (obb.axes[0] + obb.axes[1] + obb.axes[2]);
	obb.extents = glm::vec3{ length(obb.axes[0]), length(obb.axes[1]), length(obb.axes[2]) };
	obb.axes[0] = obb.axes[0] / obb.extents.x;
	obb.axes[1] = obb.axes[1] / obb.extents.y;
	obb.axes[2] = obb.axes[2] / obb.extents.z;
	obb.extents *= 0.5f;

	{
		glm::vec3 M = { 0.0f, 0.0f, 1.0f };
		float MoX = 0.0f;	// | m . x |
		float MoY = 0.0f;	// | m . y |
		float MoZ = M.z;	// m . z (not abs!)

		float MoC = obb.center.z;

		float radius = 0.0f;
		for (size_t i = 0; i < 3; i++) {
			radius += fabsf(obb.axes[i].z) * (&obb.extents.x)[i];
		}
		float obb_min = MoC - radius;
		float obb_max = MoC + radius;

		float m0 = z_far;
		float m1 = z_near;

		if (obb_min > m1 || obb_max < m0) {
			return false;
		}
	}

	{
		// Frustum normals
		const glm::vec3 M[] = {
			{ 0.0, -z_near, y_near },	// Top plane
			{ 0.0, z_near, y_near },	// Bottom plane
			{ -z_near, 0.0f, x_near },	// Right plane
			{ z_near, 0.0f, x_near },	// Left Plane
		};

		for (size_t m = 0; m < cornerCount; m++) {
			float MoX = fabsf(M[m].x);
			float MoY = fabsf(M[m].y);
			float MoZ = M[m].z;
			float MoC = dot(M[m], obb.center);

			float obb_radius = 0.0f;
			for (size_t i = 0; i < 3; i++) {
				obb_radius += fabsf(dot(M[m], obb.axes[i])) * (&obb.extents.x)[i];
			}
			float obb_min = MoC - obb_radius;
			float obb_max = MoC + obb_radius;

			float p = x_near * MoX + y_near * MoY;

			float tau_0 = z_near * MoZ - p;
			float tau_1 = z_near * MoZ + p;

			if (tau_0 < 0.0f) {
				tau_0 *= z_far / z_near;
			}
			if (tau_1 > 0.0f) {
				tau_1 *= z_far / z_near;
			}

			if (obb_min > tau_1 || obb_max < tau_0) {
				return false;
			}
		}
	}

	return true;
}

Grindstone::Renderer::CullingFrustum Grindstone::Renderer::CreateFrustum(const Grindstone::Rendering::RenderViewData& renderViewData) {
	float aspectRatio = static_cast<float>(renderViewData.renderArea.extent.x) / static_cast<float>(renderViewData.renderArea.extent.y);
	float tanFov = 1.0f / renderViewData.projectionMatrix[0][0];

	float projection_43 = renderViewData.projectionMatrix[3][2];
	float projection_33 = renderViewData.projectionMatrix[2][2];
	float nearDistance = projection_43 / (projection_33 - 1.0f);
	float farDistance = projection_43 / (projection_33 + 1.0f);

	return CullingFrustum{
		.isOrtho = (renderViewData.projectionMatrix[3][3] == 1.0f),
		.nearRight = aspectRatio * nearDistance * tanFov,
		.nearTop = nearDistance * tanFov,
		.nearDistance = -nearDistance,
		.farDistance = -farDistance,
	};
}
