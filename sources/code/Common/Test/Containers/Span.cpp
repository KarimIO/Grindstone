#include <Common/Containers/Span.hpp>

#include <gtest/gtest.h>
#include <vector>

#include <memory>
#include <utility>

namespace Grindstone::Common::Containers {
GTEST_TEST(Span, CtorPointerAndSize) {
	std::vector<int> values;
	values.resize(5);
	Grindstone::Containers::Span<int> span(&(values[1]), values.size() - 1u);

	for (size_t i = 0; i < values.size(); ++i) {
		values[i] = 16 * i;
	}

	EXPECT_EQ(span[0], 16);
	EXPECT_EQ(span[1], 32);

	int value2;
	bool isValue2Found = span.TryGet(value2, 2);
	EXPECT_EQ(isValue2Found, true);
	if (isValue2Found) {
		EXPECT_EQ(value2, 48);
	}

	EXPECT_EQ(*((&span.GetBegin()) + 3), 64);
}

GTEST_TEST(Span, GetSize) {
	std::vector<int> values;
	values.resize(5);
	Grindstone::Containers::Span<int> span(&(values[1]), values.size() - 1u);
	EXPECT_EQ(span.GetSize(), 4u);
}

GTEST_TEST(Span, GetSizeEmpty) {
	std::vector<int> values;
	values.resize(5);
	Grindstone::Containers::Span<int> span(&(values[1]), 0u);
	EXPECT_EQ(span.GetSize(), 0u);
}

GTEST_TEST(Span, TryGetAfterEnd) {
	std::vector<int> values;
	values.resize(5);
	Grindstone::Containers::Span<int> span(&(values[1]), values.size() - 1u);
	int outValue;
	EXPECT_EQ(span.TryGet(outValue, 5), false);
}

GTEST_TEST(Span, Iteration) {
	std::vector<int> values;
	values.resize(5);
	Grindstone::Containers::Span<int> span(&(values[1]), values.size() - 1u);

	size_t i = 0;
	for (auto& element : span) {
		element = i++;
	}

	i = 0;
	for (auto const& element : span) {
		EXPECT_EQ(element, i++);
	}

	EXPECT_EQ(span[0], 0);
	EXPECT_EQ(span[1], 1);
	EXPECT_EQ(span[2], 2);
	EXPECT_EQ(span[3], 3);
}

GTEST_TEST(Span, CompatibleSpans) {
	class A {
	public:
		A(int v) : value(v) {}
		int value;
		virtual int GetValue() const { return 1 * value; }
	};

	class B : public A {
	public:
		B(int v) : A(v) {}
		virtual int GetValue() const override { return 2 * value; }
	};

	std::vector<std::unique_ptr<A>> values;
	for (int i = 0; i < 5; ++i) {
		values.emplace_back(std::move(std::make_unique<B>(i * 3)));
	}
	Grindstone::Containers::Span<std::unique_ptr<A>> span(&(values[1]), values.size() - 1u);

	EXPECT_EQ(span[0]->GetValue(), 6);
	EXPECT_EQ(span[1]->GetValue(), 12);
	EXPECT_EQ(span[2]->GetValue(), 18);
	EXPECT_EQ(span[3]->GetValue(), 24);
}

}  // namespace Grindstone::Common::Containers
