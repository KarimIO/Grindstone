#ifndef _BOUNDING_H
#define _BOUNDING_H

#include <stdint.h>
#include <corecrt_math.h>

enum BoundingType : uint8_t {
	BOUNDING_SPHERE = 0,
	BOUNDING_BOX
};

class BoundingShape {
public:
	virtual void *GetData() = 0;
	virtual void TestBounding(const float [3]) = 0;
	virtual size_t GetSize() const = 0;
	virtual void Print() const = 0;
};

class BoundingSphere : public BoundingShape {
public:
	virtual void *GetData();
	virtual void TestBounding(const float [3]);
	virtual size_t GetSize() const;
	virtual void Print() const;
private:
	float radius_ = 0;
};

class BoundingBox : public BoundingShape {
public:
	virtual void *GetData();
	virtual void TestBounding(const float [3]);
	virtual size_t GetSize() const;
	virtual void Print() const;
private:
	float radius_ = 0;

	float left_ = INFINITY;
	float lower_ = INFINITY;
	float back_ = INFINITY;
	float right_ = -INFINITY;
	float upper_ = -INFINITY;
	float front_ = -INFINITY;
};

#endif