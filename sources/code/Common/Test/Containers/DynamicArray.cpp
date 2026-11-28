#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <Common/Containers/DynamicArray.hpp>
#include <Common/Memory/Allocators/DynamicAllocator.hpp>
#include <Common/Test/TestAssertSupport.hpp>

using Grindstone::Containers::DynamicArray;
using Grindstone::Memory::Allocators::DynamicAllocator;

namespace Grindstone::Common::Containers {
constexpr size_t kAllocatorSize = 1024 * 1024;

// Counts constructions/destructions so we can catch leaks, double
// destruction, and operations on objects that were never constructed.
struct Tracked {
	static inline int live = 0;
	static inline int copies = 0;
	static inline int moves = 0;
	static void Reset() { live = copies = moves = 0; }

	int value = 0;
	std::string text;

	Tracked() : value(0), text("default") {
		++live;
	}

	Tracked(int v)
		: value(v),
		text("Tracked" + std::to_string(v) + "_PaddingToDefeatSmallStringOptimization") {
		++live;
	}

	Tracked(const Tracked& o) : value(o.value), text(o.text) {
		++live;
		++copies;
	}

	Tracked(Tracked&& o) noexcept : value(o.value), text(std::move(o.text)) {
		++live;
		++moves;
	}

	Tracked& operator=(const Tracked& o) {
		value = o.value;
		text = o.text;
		++copies;
		return *this;
	}

	Tracked& operator=(Tracked&& o) noexcept {
		value = o.value;
		text = std::move(o.text);
		++moves;
		return *this;
	}

	~Tracked() {
		--live;
	}

	bool operator==(const Tracked& o) const {
		return value == o.value;
	}

	bool operator!=(const Tracked& o) const {
		return value != o.value;
	}
};

struct alignas(64) OverAligned {
	float data[16] = {};
	int id = 0;
};

class DynamicArrayTest : public ::testing::Test {
protected:
	void SetUp() override {
		Tracked::Reset();
		allocator.Initialize(kAllocatorSize);
	}

	void TearDown() override {
		EXPECT_EQ(Tracked::live, 0) << "Tracked objects leaked or were destroyed twice";
	}

	DynamicAllocator allocator;

	template <typename T>
	DynamicArray<T> Make(std::initializer_list<T> list) {
		return DynamicArray<T>(&allocator, list);
	}

	// Fill with 0..count-1 using PushBack
	template <typename T>
	DynamicArray<T> MakePushed(int count) {
		DynamicArray<T> arr(&allocator, {});
		for (int i = 0; i < count; ++i) {
			arr.PushBack(T(i));
		}
		return arr;
	}
};
	
// ----------
// Construction
// ----------

TEST_F(DynamicArrayTest, ConstructFromInitializerList) {
	auto arr = Make<int>({1, 2, 3, 4, 5});
	ASSERT_EQ(arr.GetSize(), 5u);
	EXPECT_GE(arr.GetCapacity(), 5u);
	for (int i = 0; i < 5; ++i) {
		EXPECT_EQ(arr[i], i + 1);
	}
}

TEST_F(DynamicArrayTest, ConstructFromEmptyInitializerList) {
	auto arr = Make<int>({});
	EXPECT_EQ(arr.GetSize(), 0u);
}

TEST_F(DynamicArrayTest, ConstructFromInitializerListNonTrivial) {
	{
		auto arr = Make<Tracked>({Tracked(1), Tracked(2), Tracked(3)});
		ASSERT_EQ(arr.GetSize(), 3u);
		EXPECT_EQ(arr[2].value, 3);
		EXPECT_EQ(Tracked::live, 3);
	}
	EXPECT_EQ(Tracked::live, 0);
}

TEST_F(DynamicArrayTest, ConstructFromPointerAndSize) {
	std::string source[] = {"alpha_long_string_to_avoid_sso_000000",
							"beta_long_string_to_avoid_sso_0000000",
							"gamma_long_string_to_avoid_sso_000000"};
	DynamicArray<std::string> arr(&allocator, source, 3);
	ASSERT_EQ(arr.GetSize(), 3u);
	EXPECT_EQ(arr[0], source[0]);
	EXPECT_EQ(arr[2], source[2]);
	EXPECT_GE(arr.GetCapacity(), arr.GetSize());
}

// ----------
// Push/emplace
// ----------

TEST_F(DynamicArrayTest, PushBackLvalueAndRvalue) {
	DynamicArray<Tracked> arr(&allocator, {});
	Tracked lvalue(7);
	arr.PushBack(lvalue);
	arr.PushBack(Tracked(8));
	ASSERT_EQ(arr.GetSize(), 2u);
	EXPECT_EQ(arr[0].value, 7);
	EXPECT_EQ(arr[1].value, 8);
	EXPECT_EQ(lvalue.value, 7);
}

TEST_F(DynamicArrayTest, PushBackReturnsReferenceToStoredElement) {
	DynamicArray<int> arr(&allocator, {});
	int& ref = arr.PushBack(42);
	EXPECT_EQ(&ref, &arr[0]);
	EXPECT_EQ(ref, 42);
}

TEST_F(DynamicArrayTest, PushBackGrowsAndPreservesTrivialValues) {
	auto arr = MakePushed<int>(5000);
	ASSERT_EQ(arr.GetSize(), 5000u);
	EXPECT_GE(arr.GetCapacity(), 5000u);
	for (int i = 0; i < 5000; ++i) {
		ASSERT_EQ(arr[i], i);
	}
}

TEST_F(DynamicArrayTest, PushBackGrowsAndPreservesNonTrivialValues) {
	// Forces several reallocations. ReallocRaw memcpy is only valid for
	// trivially copyable types, so this probes the non-trivial path.
	{
		auto arr = MakePushed<Tracked>(500);
		ASSERT_EQ(arr.GetSize(), 500u);
		EXPECT_EQ(Tracked::live, 500);
		for (int i = 0; i < 500; ++i) {
			ASSERT_EQ(arr[i].value, i);
			ASSERT_EQ(arr[i].text, Tracked(i).text);
		}
	}
	EXPECT_EQ(Tracked::live, 0);
}

TEST_F(DynamicArrayTest, EmplaceBackForwardsArguments) {
	EXPECT_EQ(Tracked::live, 0);
	DynamicArray<Tracked> arr(&allocator, {});
	Tracked& t = arr.EmplaceBack(99);
	EXPECT_EQ(&t, &arr[0]);
	EXPECT_EQ(t.value, 99);
	EXPECT_EQ(Tracked::live, 1);
	EXPECT_EQ(Tracked::copies, 0);
}

// ----------
// Copy
// ----------

TEST_F(DynamicArrayTest, CopyConstructorMakesIndependentDeepCopy) {
	auto original = MakePushed<Tracked>(10);
	DynamicArray<Tracked> copy(original);

	ASSERT_EQ(copy.GetSize(), original.GetSize());
	EXPECT_NE(&copy[0], &original[0]);
	for (size_t i = 0; i < 10; ++i) {
		EXPECT_EQ(copy[i].value, original[i].value);
	}

	copy[3].value = 1234;
	EXPECT_EQ(original[3].value, 3);
	EXPECT_EQ(Tracked::live, 20);
}

TEST_F(DynamicArrayTest, CopyConstructorOfEmptyArray) {
	DynamicArray<int> empty(&allocator, {});
	DynamicArray<int> copy(empty);
	EXPECT_EQ(copy.GetSize(), 0u);
}

TEST_F(DynamicArrayTest, CopyConstructorOfDefaultConstructedArray) {
	DynamicArray<int> empty;
	DynamicArray<int> copy(empty);
	EXPECT_EQ(copy.GetSize(), 0u);
}

TEST_F(DynamicArrayTest, CopyConstructedArrayCanGrow) {
	auto original = MakePushed<int>(10);
	DynamicArray<int> copy(original);
	for (int i = 0; i < 1000; ++i) {
		copy.PushBack(i);
	}

	EXPECT_EQ(copy.GetSize(), 1010u);
	EXPECT_EQ(original.GetSize(), 10u);
}

TEST_F(DynamicArrayTest, CopyAssignmentReplacesContents) {
	auto a = MakePushed<Tracked>(5);
	auto b = MakePushed<Tracked>(20);

	a = b;

	ASSERT_EQ(a.GetSize(), 20u);
	for (int i = 0; i < 20; ++i) {
		EXPECT_EQ(a[i].value, i);
	}
	EXPECT_NE(&a[0], &b[0]);
	EXPECT_EQ(Tracked::live, 40);
}

TEST_F(DynamicArrayTest, CopyAssignmentFromEmpty) {
	auto a = MakePushed<Tracked>(5);
	DynamicArray<Tracked> empty(&allocator, {});
	a = empty;
	EXPECT_EQ(a.GetSize(), 0u);
	EXPECT_EQ(Tracked::live, 0);
}

TEST_F(DynamicArrayTest, CopyAssignmentSelfAssignIsSafe) {
	auto a = MakePushed<Tracked>(5);
	auto& alias = a;
	a = alias;
	ASSERT_EQ(a.GetSize(), 5u);
	for (int i = 0; i < 5; ++i) {
		EXPECT_EQ(a[i].value, i);
	}
	EXPECT_EQ(Tracked::live, 5);
}

TEST_F(DynamicArrayTest, CopyAssignmentFromConstSourceDoesNotModifySource) {
	const auto source = MakePushed<Tracked>(5);
	DynamicArray<Tracked> dest(&allocator, {});
	dest = source;
	for (int i = 0; i < 5; ++i) {
		EXPECT_EQ(source[i].value, i);
		EXPECT_FALSE(source[i].text.empty()) << "source was moved from";
	}
}

// ----------
// Move
// ----------

TEST_F(DynamicArrayTest, MoveConstructorStealsBuffer) {
	auto source = MakePushed<Tracked>(10);
	const Tracked* originalBuffer = &source[0];
	Tracked::copies = Tracked::moves = 0;

	DynamicArray<Tracked> dest(std::move(source));

	EXPECT_EQ(dest.GetSize(), 10u);
	EXPECT_EQ(&dest[0], originalBuffer);
	EXPECT_EQ(source.GetSize(), 0u);
	EXPECT_EQ(source.GetCapacity(), 0u);
	EXPECT_EQ(Tracked::copies, 0);
	EXPECT_EQ(Tracked::moves, 0);
	EXPECT_EQ(Tracked::live, 10);
}

TEST_F(DynamicArrayTest, MoveAssignmentStealsBufferAndReturnsSelf) {
	auto a = MakePushed<Tracked>(3);
	auto b = MakePushed<Tracked>(8);
	const Tracked* bBuffer = &b[0];

	DynamicArray<Tracked>& result = (a = std::move(b));

	EXPECT_EQ(&result, &a);
	EXPECT_EQ(a.GetSize(), 8u);
	EXPECT_EQ(&a[0], bBuffer);
	EXPECT_EQ(b.GetSize(), 0u);
	EXPECT_EQ(Tracked::live, 8);
}

TEST_F(DynamicArrayTest, MoveAssignmentSelfMoveIsSafe) {
	auto a = MakePushed<Tracked>(4);
	auto& alias = a;
	a = std::move(alias);
	ASSERT_EQ(a.GetSize(), 4u);
	EXPECT_EQ(a[3].value, 3);
	EXPECT_EQ(Tracked::live, 4);
}

TEST_F(DynamicArrayTest, MovedFromArrayIsDestructibleAndReusable) {
	auto source = MakePushed<int>(4);
	DynamicArray<int> dest(std::move(source));
	// Source has no buffer but still has its allocator pointer
	EXPECT_EQ(source.GetSize(), 0u);
	source.PushBack(5);
	EXPECT_EQ(source.GetSize(), 1u);
	EXPECT_EQ(source[0], 5);
}

// ----------
// Lifetime/leaks
// ----------

TEST_F(DynamicArrayTest, DestructorDestroysElementsAndReleasesMemory) {
	{
		auto arr = MakePushed<Tracked>(100);
		EXPECT_EQ(Tracked::live, 100);
		EXPECT_GT(allocator.GetUsedSize(), 0u);
	}
	EXPECT_EQ(Tracked::live, 0);
	EXPECT_EQ(allocator.GetUsedSize(), 0u);
}

TEST_F(DynamicArrayTest, ChurnDoesNotLeakAllocatorMemory) {
	for (int round = 0; round < 50; ++round) {
		auto a = MakePushed<Tracked>(round + 1);
		DynamicArray<Tracked> b(a);
		DynamicArray<Tracked> c(std::move(b));
		a = c;
		c.Resize(1);
	}
	EXPECT_EQ(Tracked::live, 0);
	EXPECT_EQ(allocator.GetUsedSize(), 0u);
}

TEST_F(DynamicArrayTest, OverAlignedElementsAreAligned) {
	DynamicArray<OverAligned> arr(&allocator, {});
	for (int i = 0; i < 100; ++i) {
		OverAligned o;
		o.id = i;
		arr.PushBack(o);
		EXPECT_EQ(reinterpret_cast<uintptr_t>(&arr[0]) % alignof(OverAligned), 0u);
	}
	for (int i = 0; i < 100; ++i) {
		EXPECT_EQ(arr[i].id, i);
	}
}

// ----------
// Resize
// ----------

TEST_F(DynamicArrayTest, ResizeShrinkDestroysTail) {
	auto arr = MakePushed<Tracked>(10);
	arr.Resize(4);
	EXPECT_EQ(arr.GetSize(), 4u);
	EXPECT_EQ(Tracked::live, 4);
	EXPECT_EQ(arr[3].value, 3);
}

TEST_F(DynamicArrayTest, ResizeGrowDefaultConstructsNewElements) {
	auto arr = MakePushed<Tracked>(3);
	arr.Resize(200);
	ASSERT_EQ(arr.GetSize(), 200u);
	EXPECT_EQ(Tracked::live, 200);
	EXPECT_EQ(arr[2].value, 2);
	EXPECT_EQ(arr[3].text, "default");
	EXPECT_EQ(arr[199].value, 0);
}

TEST_F(DynamicArrayTest, ResizeToZeroThenPushBack) {
	auto arr = MakePushed<int>(10);
	arr.Resize(0);
	EXPECT_EQ(arr.GetSize(), 0u);
	arr.PushBack(3);
	EXPECT_EQ(arr[0], 3);
}

// ----------
// Reserve
// ----------

TEST_F(DynamicArrayTest, ReserveToExactDoesNotChangeSize) {
	auto arr = MakePushed<int>(3);
	arr.ReserveToExact(500);
	EXPECT_EQ(arr.GetSize(), 3u);
	EXPECT_EQ(arr.GetCapacity(), 500u);
	for (int i = 0; i < 3; ++i) {
		EXPECT_EQ(arr[i], i);
	}
}

TEST_F(DynamicArrayTest, ReserveToAtLeastGivesAtLeastRequested) {
	auto arr = MakePushed<int>(3);
	arr.ReserveToAtLeast(500);
	EXPECT_GE(arr.GetCapacity(), 500u);
	EXPECT_EQ(arr.GetSize(), 3u);
}

TEST_F(DynamicArrayTest, ReserveSmallerThanCapacityIsNoOp) {
	auto arr = MakePushed<int>(100);
	size_t cap = arr.GetCapacity();
	arr.ReserveToExact(1);
	EXPECT_EQ(arr.GetCapacity(), cap);
	arr.ReserveToAtLeast(1);
	EXPECT_EQ(arr.GetCapacity(), cap);
}

TEST_F(DynamicArrayTest, ReserveMoreExactAddsCapacity) {
	auto arr = MakePushed<int>(10);
	size_t cap = arr.GetCapacity();
	arr.ReserveMoreExact(50);
	EXPECT_EQ(arr.GetCapacity(), cap + 50);
	EXPECT_EQ(arr.GetSize(), 10u);
}

TEST_F(DynamicArrayTest, ReservedPushBackDoesNotMoveBuffer) {
	DynamicArray<int> arr(&allocator, {});
	arr.ReserveToExact(256);
	int* base = &arr.PushBack(0);
	for (int i = 1; i < 256; ++i) {
		arr.PushBack(i);
	}

	EXPECT_EQ(&arr[0], base);
}

// ----------
// Remove
// ----------

TEST_F(DynamicArrayTest, RemoveByIndexMiddle) {
	auto arr = MakePushed<Tracked>(5);
	arr.Remove(size_t(2));
	ASSERT_EQ(arr.GetSize(), 4u);
	EXPECT_EQ(arr[0].value, 0);
	EXPECT_EQ(arr[1].value, 1);
	EXPECT_EQ(arr[2].value, 3);
	EXPECT_EQ(arr[3].value, 4);
	EXPECT_EQ(Tracked::live, 4);
}

TEST_F(DynamicArrayTest, RemoveFirstAndLast) {
	auto arr = MakePushed<Tracked>(5);
	arr.Remove(size_t(0));
	arr.Remove(arr.GetSize() - 1);
	ASSERT_EQ(arr.GetSize(), 3u);
	EXPECT_EQ(arr[0].value, 1);
	EXPECT_EQ(arr[2].value, 3);
	EXPECT_EQ(Tracked::live, 3);
}

TEST_F(DynamicArrayTest, RemoveRange) {
	auto arr = MakePushed<Tracked>(8);
	arr.Remove(arr[2], arr[4]);
	ASSERT_EQ(arr.GetSize(), 5u);
	const int expected[] = {0, 1, 5, 6, 7};
	for (size_t i = 0; i < 5; ++i) {
		EXPECT_EQ(arr[i].value, expected[i]);
	}
	EXPECT_EQ(Tracked::live, 5);
}

TEST_F(DynamicArrayTest, RemoveOnlyElement) {
	auto arr = MakePushed<Tracked>(1);
	arr.Remove(size_t(0));
	EXPECT_EQ(arr.GetSize(), 0u);
	EXPECT_EQ(Tracked::live, 0);
}

// ----------
// Append list
// ----------

TEST_F(DynamicArrayTest, AppendListBackAddsToEnd) {
	auto arr = MakePushed<int>(3);
	arr.AppendListBack({10, 11, 12});
	ASSERT_EQ(arr.GetSize(), 6u);
	const int expected[] = {0, 1, 2, 10, 11, 12};
	for (size_t i = 0; i < 6; ++i) {
		EXPECT_EQ(arr[i], expected[i]);
	}
}

TEST_F(DynamicArrayTest, AppendListBackLargeForcesGrowth) {
	auto arr = MakePushed<Tracked>(2);
	arr.AppendListBack({Tracked(10), Tracked(11), Tracked(12), Tracked(13), Tracked(14)});
	ASSERT_EQ(arr.GetSize(), 7u);
	EXPECT_EQ(arr[6].value, 14);
	EXPECT_EQ(Tracked::live, 7);
}

TEST_F(DynamicArrayTest, AppendListInsertsAtOffsetMiddle) {
	auto arr = MakePushed<int>(5);
	arr.AppendList({100, 101}, 2);
	ASSERT_EQ(arr.GetSize(), 7u);
	const int expected[] = {0, 1, 100, 101, 2, 3, 4};
	for (size_t i = 0; i < 7; ++i) {
		EXPECT_EQ(arr[i], expected[i]);
	}
}

TEST_F(DynamicArrayTest, AppendListInsertsAtFront) {
	auto arr = MakePushed<int>(3);
	arr.AppendList({100, 101}, 0);
	ASSERT_EQ(arr.GetSize(), 5u);
	const int expected[] = {100, 101, 0, 1, 2};
	for (size_t i = 0; i < 5; ++i) {
		EXPECT_EQ(arr[i], expected[i]);
	}
}

// ----------
// Iteration
// ----------

TEST_F(DynamicArrayTest, RangeForVisitsEveryElement) {
	auto arr = MakePushed<int>(10);
	int sum = 0, count = 0;
	for (int v : arr) {
		sum += v;
		++count;
	}
	EXPECT_EQ(count, 10);
	EXPECT_EQ(sum, 45);
}

TEST_F(DynamicArrayTest, ConstRangeForVisitsEveryElement) {
	const auto arr = MakePushed<int>(10);
	int sum = 0, count = 0;
	for (int v : arr) {
		sum += v;
		++count;
	}
	EXPECT_EQ(count, 10);
	EXPECT_EQ(sum, 45);
}

TEST_F(DynamicArrayTest, CBeginCEndVisitEveryElement) {
	auto arr = MakePushed<int>(10);
	int count = 0;

	for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
		++count;
	}
	EXPECT_EQ(count, 10);
}

TEST_F(DynamicArrayTest, EmptyArrayIteratesZeroTimes) {
	DynamicArray<int> arr(&allocator, {});
	int count = 0;
	for (int v : arr) {
		(void)v;
		++count;
	}
	EXPECT_EQ(count, 0);
}

TEST_F(DynamicArrayTest, RangeForCanMutate) {
	auto arr = MakePushed<int>(5);
	for (int& v : arr) {
		v *= 2;
	}

	for (int i = 0; i < 5; ++i) {
		EXPECT_EQ(arr[i], i * 2);
	}
}

TEST_F(DynamicArrayTest, ReverseIterationVisitsInReverseOrder) {
	auto arr = MakePushed<int>(5);
	std::vector<int> seen;
	for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
		seen.push_back(*it);
	}

	ASSERT_EQ(seen.size(), 5u);
	for (int i = 0; i < 5; ++i) {
		EXPECT_EQ(seen[i], 4 - i);
	}
}

// ----------
// Accessors
// ----------

TEST_F(DynamicArrayTest, GetBeginAndGetEnd) {
	auto arr = MakePushed<int>(4);
	EXPECT_EQ(arr.GetBegin(), 0);
	EXPECT_EQ(&arr.GetBegin(), &arr[0]);
}

TEST_F(DynamicArrayTest, TryGetReturnsValueWhenInRange) {
	auto arr = MakePushed<int>(4);
	int out = -1;
	EXPECT_TRUE(arr.TryGet(out, 2));
	EXPECT_EQ(out, 2);
}

TEST_F(DynamicArrayTest, TryGetFailsWhenOutOfRange) {
	auto arr = MakePushed<int>(4);
	int out = -1;
	EXPECT_FALSE(arr.TryGet(out, 4));
	EXPECT_EQ(out, -1);
}

TEST_F(DynamicArrayTest, GetMatchesIndexOperator) {
	auto arr = MakePushed<int>(4);
	for (size_t i = 0; i < 4; ++i) {
		EXPECT_EQ(&arr.Get(i), &arr[i]);
	}
}

TEST_F(DynamicArrayTest, EqualityComparesSizeAndElements) {
	auto a = MakePushed<int>(4);
	auto b = MakePushed<int>(4);
	auto c = MakePushed<int>(5);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a == c);
	b[2] = 99;
	EXPECT_FALSE(a == b);
}

TEST_F(DynamicArrayTest, EqualityOfEmptyArrays) {
	DynamicArray<int> a(&allocator, {});
	DynamicArray<int> b(&allocator, {});
	EXPECT_TRUE(a == b);
}

// ----------
// Death
// ----------

#ifndef NDEBUG
TEST_F(DynamicArrayTest, OutOfRangeIndexAsserts) {
	auto arr = MakePushed<int>(2);
	ScopedAssertCapture capture(ScopedAssertCapture::Mode::Throw);
	EXPECT_THROW((void)arr[2], AssertFailure);
	EXPECT_TRUE(capture.Contains("index is invalid"));
}
#endif
}
