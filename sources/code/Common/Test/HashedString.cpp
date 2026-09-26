#include <Common/HashedString.hpp>

#include <gtest/gtest.h>
#include <memory>

enum class TestEnum { MyError, OtherError };

namespace Grindstone::Common {
TEST(HashedString, StringRecovery) {
	std::unique_ptr<HashedString::HashMap> hashedMap =
		std::make_unique<HashedString::HashMap>();
	Grindstone::HashedString::SetHashMap(hashedMap.get());
	Grindstone::HashedString result = "My Hash"_hash;

	EXPECT_EQ(result.ToString(), "My Hash");
	Grindstone::HashedString::SetHashMap(nullptr);
}

TEST(HashedString, HashConsistency) {
	std::unique_ptr<HashedString::HashMap> hashedMap =
		std::make_unique<HashedString::HashMap>();
	Grindstone::HashedString::SetHashMap(hashedMap.get());
	Grindstone::HashedString result = "My Hash"_hash;

	EXPECT_EQ(result.GetHash(), 7940637730787906758);
	Grindstone::HashedString::SetHashMap(nullptr);
}

TEST(HashedString, Uniqueness) {
	std::unique_ptr<HashedString::HashMap> hashedMap =
		std::make_unique<HashedString::HashMap>();
	Grindstone::HashedString::SetHashMap(hashedMap.get());
	Grindstone::HashedString hash1 = "My Hash"_hash;
	Grindstone::HashedString hash2 = "Your Hash"_hash;

	EXPECT_EQ(hash1.ToString(), "My Hash");
	EXPECT_EQ(hash2.ToString(), "Your Hash");
	EXPECT_NE(hash1.GetHash(), hash2.GetHash());
	Grindstone::HashedString::SetHashMap(nullptr);
}
}  // namespace Grindstone::Common
