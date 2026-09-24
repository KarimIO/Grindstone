#include <Common/Memory/SmartPointers/UniquePtr.hpp>

#include <utility>

#include <gtest/gtest.h>

struct TrackingAllocator {
	size_t allocations = 0;
	size_t deallocations = 0;

	template <typename T, typename... Args>
	T* Allocate(Args&&... args) {
		++allocations;
		return new T(std::forward<Args>(args)...);
	}

	template <typename T>
	void Deallocate(T* ptr) {
		++deallocations;
		delete ptr;
	}
};

struct TestClass {
	TestClass(int v) : i(v) {}
	int i = 0;
};

namespace Grindstone::Memory::SmartPointers {
TEST(UniquePtr, AllocatorsAndDeletesAtScope) {
	TrackingAllocator allocator;

	{
		Grindstone::UniquePtr<TestClass> uptr(
			allocator.Allocate<TestClass>(42),
			[&allocator](void* p) { allocator.Deallocate(p); });
		ASSERT_EQ(allocator.allocations, 1);
		ASSERT_EQ(allocator.deallocations, 0);
		ASSERT_EQ(uptr->i, 42);
	}

	ASSERT_EQ(allocator.deallocations, 1);
}

TEST(UniquePtr, Move) {
	TrackingAllocator allocator;

	{
		Grindstone::UniquePtr<TestClass> uptr1(
			allocator.Allocate<TestClass>(42),
			[&allocator](void* p) { allocator.Deallocate(p); });
		ASSERT_EQ(allocator.allocations, 1);
		ASSERT_EQ(allocator.deallocations, 0);
		ASSERT_EQ(uptr1->i, 42);

		Grindstone::UniquePtr<TestClass> uptr2(std::move(uptr1));
		ASSERT_EQ(allocator.allocations, 1);
		ASSERT_EQ(allocator.deallocations, 0);
		ASSERT_EQ(uptr2->i, 42);
	}

	ASSERT_EQ(allocator.deallocations, 1);
}
}  // namespace Grindstone::Memory::SmartPointers
