#include <Common/Memory/Allocators/PoolAllocator.hpp>

#include <gtest/gtest.h>

constexpr size_t kAllocatorSize = 4u;

struct TestClass {
	int a = 4231;
	int b = 5585;

	TestClass() = default;
	TestClass(int a, int b) : a(a), b(b) {}
};

namespace Grindstone::Common::Memory::Allocators {
TEST(PoolAllocator, DefaultUsedSizeWhenEmpty) {
	Grindstone::Memory::Allocators::PoolAllocator<TestClass> allocator;
	allocator.Initialize(kAllocatorSize);
	ASSERT_EQ(allocator.GetUsedCount(), 0);
}

TEST(PoolAllocator, AllocateCtor) {
	Grindstone::Memory::Allocators::PoolAllocator<TestClass> allocator;
	allocator.Initialize(kAllocatorSize);

	TestClass* ptr1 = allocator.Allocate(999, 124);
	ASSERT_NE(ptr1, nullptr);
	TestClass* ptr2 = allocator.Allocate();
	ASSERT_NE(ptr2, nullptr);
	ASSERT_NE(ptr1, ptr2);

	ASSERT_EQ(ptr1->a, 999);
	ASSERT_EQ(ptr1->b, 124);
	ASSERT_EQ(ptr2->a, 4231);
	ASSERT_EQ(ptr2->b, 5585);
}

TEST(PoolAllocator, AllocateOverLimitFails) {
	Grindstone::Memory::Allocators::PoolAllocator<TestClass> allocator;
	allocator.Initialize(kAllocatorSize);

	TestClass* ptr1 = allocator.Allocate(999, 124);
	ASSERT_NE(ptr1, nullptr);
	TestClass* ptr2 = allocator.Allocate();
	ASSERT_NE(ptr2, nullptr);
	TestClass* ptr3 = allocator.Allocate(6, 2);
	ASSERT_NE(ptr3, nullptr);
	TestClass* ptr4 = allocator.Allocate(22, 6124);
	ASSERT_NE(ptr4, nullptr);

	TestClass* ptr5 = allocator.Allocate(997, 5123);
	ASSERT_EQ(ptr5, nullptr);
}

TEST(PoolAllocator, AllocateAndFree) {
	Grindstone::Memory::Allocators::PoolAllocator<TestClass> allocator;
	allocator.Initialize(kAllocatorSize);

	TestClass* ptr1 = allocator.Allocate(999, 124);
	ASSERT_NE(ptr1, nullptr);
	ASSERT_EQ(allocator.GetUsedCount(), 1);

	allocator.Deallocate(ptr1);
	ASSERT_EQ(allocator.GetUsedCount(), 0);

	TestClass* ptr2 = allocator.Allocate(999, 124);
	ASSERT_NE(ptr2, nullptr);
	ASSERT_EQ(allocator.GetUsedCount(), 1);

}

TEST(PoolAllocator, AllocateAndFreeOverLimitFails) {
	Grindstone::Memory::Allocators::PoolAllocator<TestClass> allocator;
	allocator.Initialize(kAllocatorSize);

	TestClass* ptr1 = allocator.Allocate(999, 124);
	ASSERT_NE(ptr1, nullptr);
	TestClass* ptr2 = allocator.Allocate();
	ASSERT_NE(ptr2, nullptr);
	TestClass* ptr3 = allocator.Allocate(6, 2);
	ASSERT_NE(ptr3, nullptr);
	TestClass* ptr4 = allocator.Allocate(22, 6124);
	ASSERT_NE(ptr4, nullptr);

	ASSERT_EQ(allocator.GetUsedCount(), 4);
	allocator.Deallocate(ptr4);
	ASSERT_EQ(allocator.GetUsedCount(), 3);
	allocator.Deallocate(ptr3);
	ASSERT_EQ(allocator.GetUsedCount(), 2);

	TestClass* ptr5 = allocator.Allocate(997, 5123);
	ASSERT_NE(ptr5, nullptr);
	ASSERT_EQ(ptr3, ptr5);
	ASSERT_EQ(allocator.GetUsedCount(), 3);
	TestClass* ptr6 = allocator.Allocate(51, 5);
	ASSERT_NE(ptr6, nullptr);
	ASSERT_EQ(ptr4, ptr6);

	ASSERT_EQ(allocator.GetUsedCount(), 4);
	allocator.Deallocate(ptr6);
	ASSERT_EQ(allocator.GetUsedCount(), 3);

	TestClass* ptr7 = allocator.Allocate();
	ASSERT_NE(ptr7, nullptr);
	ASSERT_EQ(allocator.GetUsedCount(), 4);

	TestClass* ptr8 = allocator.Allocate();
	ASSERT_EQ(ptr8, nullptr);
	ASSERT_EQ(allocator.GetUsedCount(), 4);

	ASSERT_EQ(ptr6, ptr4);
	ASSERT_EQ(ptr7, ptr4);
}
}  // namespace Grindstone::Common::Memory::Allocators
