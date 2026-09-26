#include <Common/Memory/Allocators/DynamicAllocator.hpp>

#include <gtest/gtest.h>

constexpr size_t kAllocatorSize = 128u;
constexpr size_t kAllocationHeaderSize =
	sizeof(Grindstone::Memory::Allocators::DynamicAllocator::AllocationHeader);
constexpr size_t kFreeHeaderSize =
	sizeof(Grindstone::Memory::Allocators::DynamicAllocator::FreeHeader);
constexpr size_t kFreeHeaderAlign =
	alignof(Grindstone::Memory::Allocators::DynamicAllocator::FreeHeader);

static size_t CalculatePadding(size_t baseAddress, size_t alignment) {
	const size_t remainder = baseAddress % alignment;
	return remainder == 0 ? 0 : alignment - remainder;
}

static size_t CalculatePaddingWithHeader(size_t baseAddress, size_t alignment,
										 size_t headerSize) {
	size_t padding = CalculatePadding(baseAddress, alignment);
	size_t neededSpace = headerSize;

	if (padding < neededSpace) {
		// Find next aligned address
		neededSpace -= padding;

		if (neededSpace % alignment > 0) {
			padding += alignment * (1 + (neededSpace / alignment));
		} else {
			padding += alignment * (neededSpace / alignment);
		}
	}

	return padding;
}

static size_t CalculateAllocatedSize(size_t baseAddress, size_t size,
									 size_t alignment) {
	size_t padding = CalculatePaddingWithHeader(baseAddress, alignment,
												kAllocationHeaderSize);
	std::size_t requiredSize = size + padding;
	return (requiredSize + kFreeHeaderAlign - 1) & ~(kFreeHeaderAlign - 1);
}

struct TestStruct {
	int v = 42;
};

namespace Grindstone::Common::Memory::Allocators {
TEST(DynamicAllocator, DefaultUsedSizeWhenEmpty) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	ASSERT_EQ(allocator.GetUsedSize(), 0);
}

TEST(DynamicAllocator, AllocateRaw) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	void* baseAddr = allocator.GetMemory();
	size_t allocatedSize = CalculateAllocatedSize(reinterpret_cast<size_t>(baseAddr),
												  sizeof(int), alignof(int));

	void* ptr = allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");
	ASSERT_NE(ptr, nullptr);

	int* value = reinterpret_cast<int*>(ptr);
	*value = 32;

	ASSERT_EQ(*value, 32);
	ASSERT_EQ(allocator.GetUsedSize(), allocatedSize);
}

TEST(DynamicAllocator, TwoAllocsDifferentAddress) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	void* baseAddr = allocator.GetMemory();
	void* firstPtr = allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");
	ASSERT_NE(firstPtr, nullptr);
	void* secondPtr = allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");
	ASSERT_NE(secondPtr, nullptr);

	size_t allocatedSize1 = CalculateAllocatedSize(
		reinterpret_cast<size_t>(baseAddr), sizeof(int), alignof(int));

	size_t allocatedSize2 = CalculateAllocatedSize(
		reinterpret_cast<size_t>(baseAddr) + allocatedSize1, sizeof(int),
		alignof(int));

	ASSERT_NE(firstPtr, secondPtr);
	ASSERT_EQ(allocator.GetUsedSize(), allocatedSize1 + allocatedSize2);
}

// This is a test of free - we verify that oncee we free we get a new value
// from the same spot. It's the only real way to check free.
TEST(DynamicAllocator, FreeAndAllocatesFromSameSpotWithOneAlloc) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	void* baseAddr = allocator.GetMemory();

	size_t size = CalculateAllocatedSize(reinterpret_cast<size_t>(baseAddr),
											 sizeof(int), alignof(int));

	void* originalPtr =
		allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");

	ASSERT_EQ(allocator.GetUsedSize(), size);
	ASSERT_EQ(allocator.Free(originalPtr), true);
	ASSERT_EQ(allocator.GetUsedSize(), 0u);

	void* secondPtr =
		allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");

	ASSERT_EQ(originalPtr, secondPtr);
	ASSERT_EQ(allocator.GetUsedSize(), size);
}

// This is a test of free - we verify that oncee we free we get a new value
// from the same spot. It's the only real way to check free.
TEST(DynamicAllocator, FreeAndAllocatesFromSameSpotWithMultipleAllocs) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	void* baseAddr = allocator.GetMemory();

	size_t allocatedSize1 = CalculateAllocatedSize(
		reinterpret_cast<size_t>(baseAddr), sizeof(int), alignof(int));

	size_t allocatedSize2 = CalculateAllocatedSize(
		reinterpret_cast<size_t>(baseAddr) + allocatedSize1, sizeof(int),
		alignof(int));

	void* originalPtr =
		allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");

	void* secondPtr =
		allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");

	ASSERT_EQ(allocator.GetUsedSize(), allocatedSize1 + allocatedSize2);
	ASSERT_EQ(allocator.Free(originalPtr), true);
	ASSERT_EQ(allocator.GetUsedSize(), allocatedSize1);

	void* thirdPtr =
		allocator.AllocateRaw(sizeof(int), alignof(int), "My Int");

	ASSERT_EQ(originalPtr, thirdPtr);
	ASSERT_EQ(allocator.GetUsedSize(), allocatedSize1 + allocatedSize2);
}

TEST(DynamicAllocator, AllocateAndCtor) {
	Grindstone::Memory::Allocators::DynamicAllocator allocator;
	allocator.Initialize(kAllocatorSize);
	void* baseAddr = allocator.GetMemory();

	TestStruct* ts = allocator.AllocateRaw<TestStruct>();
	ASSERT_EQ(ts->v, 42);

	size_t expectedSize =
		CalculateAllocatedSize(reinterpret_cast<size_t>(baseAddr), sizeof(TestStruct), alignof(TestStruct));

	ASSERT_EQ(allocator.GetUsedSize(), expectedSize);
}
}  // namespace Grindstone::Memory::Allocators
