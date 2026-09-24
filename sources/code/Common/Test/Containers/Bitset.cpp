#include <gtest/gtest.h>

#include <Common/Containers/Bitset.hpp>

namespace Grindstone::Common::Containers {
TEST(Bitset, RetainValue) {
	Grindstone::Containers::Bitset<4> bitset;
	bitset.Set();

	EXPECT_EQ(bitset.GetWord(0), 15u);
}

// TODO: Add these flags
// construction / initial state
// setting / clearing bits
// testing bits
// Set Reset, Flip
// all - zero / all - one cases
// first / last bit
// bitwise operations
// counting bits
// iteration
// sizes that cross word boundaries(e.g.1, 7, 8, 31, 32, 33, 63, 64, 65 bits)

// individual flags
// combining flags
// testing flags
// removing flags
// empty / full flag sets
// bitwise operators
// enum / flag conversion behavior
// invalid / unused bits where relevant

}  // namespace Grindstone::Common::Containers
