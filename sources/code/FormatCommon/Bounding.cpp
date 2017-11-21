#include "Bounding.hpp"
#include <cmath>
#include <iostream>

void *BoundingSphere::GetData() {
	return &radius_;
}

void BoundingSphere::TestBounding(const float vertex[3]) {
	float distance = sqrt(vertex[0]*vertex[0] + vertex[1]*vertex[1] + vertex[2]*vertex[2]);
	radius_ = (distance > radius_) ? distance : radius_;
}

size_t BoundingSphere::GetSize() const {
	return sizeof(float);
}

void BoundingSphere::Print() const {
	std::cout << "Bounding Sphere: " << std::endl
					<< "\tRadius: " << radius_ << std::endl;
}

bool BoundingSphere::TestCamera(float near, float far, float fov, float sphere_factor_x, float sphere_factor_y, float aspect, glm::vec3 origin, glm::vec3 eye, glm::vec3 forward, glm::vec3 up, glm::vec3 right) {
	glm::vec3 v = origin - eye;
	float pc_z = glm::dot(v, forward);
	if (pc_z < near - radius_ || pc_z > far + radius_) {
		return false;
	}

	float pc_y = glm::dot(v, up);
	pc_z *= fov;
	float d = sphere_factor_y * radius_;
	if (pc_y < -pc_z - d || pc_y > pc_z + d) {
		return false;
	}

	float pc_x = glm::dot(v, right);
	pc_z *= aspect;
	d = sphere_factor_x * radius_;
	if (pc_x < -pc_z - d || pc_x > pc_z + d) {
		return false;
	}
	
	return true;
}

void *BoundingBox::GetData() {
	return BoundingSphere::GetData();
}

size_t BoundingBox::GetSize() const {
	return sizeof(float) * 7;
}

void BoundingBox::Print() const {
	BoundingSphere::Print();
	std::cout << "Bounding Box: " << std::endl
		<< "\tLeft: " << left_
		<< "\tRight: " << right_ << std::endl 
		<< "\tTop: " << upper_
		<< "\tBottom: " << lower_ << std::endl
		<< "\tFront: " << front_
		<< "\tBack: " << back_ << std::endl;
}

void BoundingBox::TestBounding(const float vertex[3]) {
	BoundingSphere::TestBounding(vertex);

	left_ = (vertex[0] < left_) ? vertex[0] : left_;
	lower_ = (vertex[1] < lower_) ? vertex[1] : lower_;
	back_ = (vertex[2] < back_) ? vertex[2] : back_;
	right_ = (vertex[0] > right_) ? vertex[0] : right_;
	upper_ = (vertex[1] > upper_) ? vertex[1] : upper_;
	front_ = (vertex[2] > front_) ? vertex[2] : front_;
}

bool BoundingBox::TestCamera(float near, float far, float fov, float sphere_factor_x, float sphere_factor_y, float aspect, glm::vec3 origin, glm::vec3 eye, glm::vec3 forward, glm::vec3 up, glm::vec3 right) {
	// Simple bounding sphere
	if (!BoundingSphere::TestCamera(near, far, fov, sphere_factor_x, sphere_factor_y, aspect, origin, eye, forward, up, right))
		return false;


	// Complex Bounding Box
	glm::vec3 box_vertices[8] = {
		glm::vec3(left_,  lower_, back_),
		glm::vec3(left_,  lower_, front_),
		glm::vec3(left_,  upper_, back_),
		glm::vec3(left_,  upper_, front_),
		glm::vec3(right_, lower_, back_),
		glm::vec3(right_, lower_, front_),
		glm::vec3(right_, upper_, back_),
		glm::vec3(right_, upper_, front_),
	};

	for (glm::vec3 &vertex : box_vertices) {
		if (TestPoint(near, far, fov, aspect, origin + vertex, eye, forward, up, right))
			return true;
	}

	return false;
}

bool BoundingBox::TestPoint(float near, float far, float fov, float aspect, glm::vec3 origin, glm::vec3 eye, glm::vec3 forward, glm::vec3 up, glm::vec3 right) {
	glm::vec3 v = origin - eye;
	float pc_z = glm::dot(v, forward);
	if (pc_z < near || pc_z > far) {
		return false;
	}

	float pc_y = glm::dot(v, up);
	float hh = pc_z * fov;
	if (pc_y < -hh || pc_y > hh) {
		return false;
	}

	float pc_x = glm::dot(v, right);
	float wh = hh * aspect;
	if (-wh > pc_x || pc_x > wh) {
		return false;
	}

	return true;
}
