#include <Common/Containers/Iterators.hpp>

#include <gtest/gtest.h>

namespace Grindstone::Common::Containers {
TEST(Iterators, RetainValue) {
	std::vector<int> values;
	values.resize(5);
	for (int i = 0; i < values.size(); ++i) {
		values[i] = 16 * i;
	}

	int multiplier = 0; 
	for (
		Grindstone::Containers::ArrayIterator<int> it(&values[0]);
		 it < Grindstone::Containers::ArrayIterator<int>(values.end()._Ptr);
		 ++it
	) {
		EXPECT_EQ(*it, multiplier * 16);
		++multiplier;
	}
}
}  // namespace Grindstone::Common::Containers
