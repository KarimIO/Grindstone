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

void *BoundingBox::GetData() {
	return &radius_;
}

size_t BoundingBox::GetSize() const {
	return sizeof(float) * 7;
}

void BoundingBox::Print() const {
	std::cout << "Bounding Box: " << std::endl
		<< "\tRadius: " << radius_ << std::endl
		<< "Bounding Box: " << std::endl
		<< "\tLeft: " << left_
		<< "\tRight: " << right_ << std::endl 
		<< "\tTop: " << upper_
		<< "\tBottom: " << lower_ << std::endl
		<< "\tFront: " << front_
		<< "\tBack: " << back_ << std::endl;
}

void BoundingBox::TestBounding(const float vertex[3]) {
	float distance = sqrt(vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2]);
	radius_ = (distance > radius_) ? distance : radius_;

	left_ = (vertex[0] < left_) ? vertex[0] : left_;
	lower_ = (vertex[1] < lower_) ? vertex[1] : lower_;
	back_ = (vertex[2] < back_) ? vertex[2] : back_;
	right_ = (vertex[0] > right_) ? vertex[0] : right_;
	upper_ = (vertex[1] > upper_) ? vertex[1] : upper_;
	front_ = (vertex[2] > front_) ? vertex[2] : front_;
}