#include <Common/Memory/Allocators/LinearAllocator.hpp>

#include <gtest/gtest.h>

constexpr size_t kAllocatorSize = 128u;

struct TestClass {
	int a = 4231;
	int b = 5585;

	TestClass() = default;
	TestClass(int a, int b) : a(a), b(b) {}
};

namespace Grindstone::Common::Memory::Allocators {
TEST(LinearAllocator, DefaultUsedSizeWhenEmpty) {
	Grindstone::Memory::Allocators::LinearAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	ASSERT_EQ(allocator.GetUsedSize(), 0);
}

TEST(LinearAllocator, AllocateRaw) {
	Grindstone::Memory::Allocators::LinearAllocator allocator;
	allocator.Initialize(kAllocatorSize);

	void* ptr1 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr1, nullptr);
	void* ptr2 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr2, nullptr);
	ASSERT_NE(ptr1, ptr2);

	int* int1 = static_cast<int*>(ptr1);
	int* int2 = static_cast<int*>(ptr2);
	*int1 = 5123;
	*int2 = 41;

	ASSERT_EQ(*int1, 5123);
	ASSERT_EQ(*int2, 41);
}

TEST(LinearAllocator, AllocateClear) {
	Grindstone::Memory::Allocators::LinearAllocator allocator;
	allocator.Initialize(kAllocatorSize);

	void* ptr1 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr1, nullptr);
	void* ptr2 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr2, nullptr);
	ASSERT_NE(ptr1, ptr2);

	int* int1 = static_cast<int*>(ptr1);
	int* int2 = static_cast<int*>(ptr2);
	*int1 = 5123;
	*int2 = 41;

	ASSERT_EQ(*int1, 5123);
	ASSERT_EQ(*int2, 41);

	allocator.Clear();

	void* ptr3 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr3, nullptr);
	void* ptr4 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr4, nullptr);
	ASSERT_NE(ptr3, ptr4);

	ASSERT_EQ(ptr1, ptr3);
	ASSERT_EQ(ptr2, ptr4);

	ASSERT_EQ(*int1, 5123);
	ASSERT_EQ(*int2, 41);
}

TEST(LinearAllocator, AllocateClearAndZero) {
	Grindstone::Memory::Allocators::LinearAllocator allocator;
	allocator.Initialize(kAllocatorSize);

	void* ptr1 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr1, nullptr);
	void* ptr2 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr2, nullptr);
	ASSERT_NE(ptr1, ptr2);

	int* int1 = static_cast<int*>(ptr1);
	int* int2 = static_cast<int*>(ptr2);
	*int1 = 5123;
	*int2 = 41;

	ASSERT_EQ(*int1, 5123);
	ASSERT_EQ(*int2, 41);

	allocator.ClearAndZero();

	void* ptr3 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr3, nullptr);
	void* ptr4 = allocator.AllocateRaw(sizeof(int), alignof(int));
	ASSERT_NE(ptr4, nullptr);
	ASSERT_NE(ptr3, ptr4);

	ASSERT_EQ(ptr1, ptr3);
	ASSERT_EQ(ptr2, ptr4);

	ASSERT_EQ(*int1, 0);
	ASSERT_EQ(*int2, 0);
}

TEST(LinearAllocator, AllocateCtor) {
	Grindstone::Memory::Allocators::LinearAllocator allocator;
	allocator.Initialize(kAllocatorSize);

	TestClass* ptr1 = allocator.Allocate<TestClass>(999, 124);
	ASSERT_NE(ptr1, nullptr);
	TestClass* ptr2 = allocator.Allocate<TestClass>();
	ASSERT_NE(ptr2, nullptr);
	ASSERT_NE(ptr1, ptr2);

	ASSERT_EQ(ptr1->a, 999);
	ASSERT_EQ(ptr1->b, 124);
	ASSERT_EQ(ptr2->a, 4231);
	ASSERT_EQ(ptr2->b, 5585);
}
}  // namespace Grindstone::Common::Memory::Allocators
