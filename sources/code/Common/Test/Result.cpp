#include <Common/Result.hpp>

#include <gtest/gtest.h>
#include <string>
#include <utility>

enum class TestEnum {
	MyError,
	OtherError
};

namespace Grindstone::Common {
TEST(Result, Value) {
	Grindstone::Result<std::string, TestEnum> result =
		std::move(std::string("My Value"));

	EXPECT_EQ(result.HasError(), false);
	EXPECT_EQ(result.HasValue(), true);
	EXPECT_EQ(result.GetValue(), "My Value");
}

TEST(Result, Error) {
	Grindstone::Result<std::string, TestEnum> result = TestEnum::MyError;

	EXPECT_EQ(result.HasError(), true);
	EXPECT_EQ(result.HasValue(), false);

	EXPECT_EQ(result.GetError(), TestEnum::MyError);

	result = TestEnum::OtherError;
	EXPECT_EQ(result.HasError(), true);
	EXPECT_EQ(result.HasValue(), false);
	EXPECT_EQ(result.GetError(), TestEnum::OtherError);
}
}  // namespace Grindstone::Common
