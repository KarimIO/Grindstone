#include <Common/Containers/ResourceCache.hpp>
#include <Common/DeferredDeletionQueue.hpp>
#include <Common/Test/TestAssertSupport.hpp>

#include <atomic>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace Grindstone::Common::Containers {
struct Resource {
	explicit Resource(int key) : key(key) {}
	int key;
	std::atomic<bool> alive{true};
};

using Cache = Grindstone::Containers::ResourceCache<int, Resource*>;
using Ref = Cache::Ref;

class ResourceCacheTest : public ::testing::Test {
protected:
	void SetUp() override {
		cache.SetFactory([this](const int& key) {
			std::lock_guard lock(bookkeeping);
			++factoryCalls;
			lastKey = key;
			Resource* r = new Resource(key);
			created.push_back(r);
			return r;
		});

		// The deleter only records. Memory is freed in TearDown so the
		// threaded tests can safely inspect "alive" on dead resources.
		cache.SetDeleter([this](Resource* r) {
			std::lock_guard lock(bookkeeping);
			deleted.push_back(r);
			if (r) {
				r->alive.store(false);
			}
		});
	}

	void TearDown() override {
		cache.TrimImmediately();  // cache destructor asserts it is empty
		for (Resource* r : created) {
			delete r;
		}
	}

	size_t DeletedCount() {
		std::lock_guard lock(bookkeeping);
		return deleted.size();
	}

	bool WasDeleted(Resource* r) {
		std::lock_guard lock(bookkeeping);
		for (Resource* d : deleted) {
			if (d == r) {
				return true;
			}
		}
		return false;
	}

	Cache cache;
	std::mutex bookkeeping;
	std::vector<Resource*> created;
	std::vector<Resource*> deleted;
	int factoryCalls = 0;
	int lastKey = -1;
};

// ----------
// Ref Basics
// ----------

TEST_F(ResourceCacheTest, DefaultRefIsInvalid) {
	Ref ref;
	EXPECT_FALSE(ref.IsValid());
	EXPECT_EQ(ref.Get(), nullptr);
	EXPECT_EQ(ref.operator->(), nullptr);
}

TEST_F(ResourceCacheTest, NullptrRefIsInvalid) {
	Ref ref(nullptr);
	EXPECT_FALSE(ref.IsValid());
	EXPECT_EQ(ref.Get(), nullptr);
}

TEST_F(ResourceCacheTest, RefFromCacheIsValidAndExposesValue) {
	Ref ref = cache.GetOrCreate(5);
	ASSERT_TRUE(ref.IsValid());
	ASSERT_NE(ref.Get(), nullptr);
	EXPECT_EQ(ref.Get()->key, 5);
	EXPECT_EQ(ref->key, 5);
	EXPECT_EQ(ref.Get(), ref.operator->());
}

TEST_F(ResourceCacheTest, DefaultRefsCompareEqual) {
	Ref a, b;
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);
}

TEST_F(ResourceCacheTest, RefsToSameKeyAreEqualAndDifferentKeysAreNot) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(1);
	Ref c = cache.GetOrCreate(2);
	EXPECT_TRUE(a == b);
	EXPECT_TRUE(a != c);
	EXPECT_FALSE(a == Ref());
}

// ----------
// Ref Copying
// ----------

TEST_F(ResourceCacheTest, CopyConstructedRefKeepsEntryAlive) {
	Resource* raw = nullptr;
	{
		Ref a = cache.GetOrCreate(1);
		raw = a.Get();
		{
			Ref b(a);
			EXPECT_EQ(b.Get(), raw);
		}
		// b gone, a still holds a reference
		cache.TrimImmediately();
		EXPECT_FALSE(WasDeleted(raw));
	}
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, CopyOfDefaultRefDoesNotCrash) {
	Ref empty;
	Ref copy(empty);
	EXPECT_FALSE(copy.IsValid());
}

TEST_F(ResourceCacheTest, RefsInVectorSurviveGrowthWithNullEntries) {
	std::vector<Ref> refs;
	refs.push_back(Ref());
	refs.push_back(cache.GetOrCreate(1));
	refs.push_back(Ref());
	for (int i = 0; i < 50; ++i) {
		refs.push_back(cache.GetOrCreate(i));  // forces reallocation/copies
	}
	EXPECT_TRUE(refs[1].IsValid());
	EXPECT_FALSE(refs[0].IsValid());
}

TEST_F(ResourceCacheTest, CopyAssignmentSharesEntryAndReleasesOldOne) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(2);
	Resource* first = a.Get();
	Resource* second = b.Get();

	a = b;

	EXPECT_EQ(a.Get(), second);
	EXPECT_TRUE(a == b);

	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(first));	   // a no longer holds key 1
	EXPECT_FALSE(WasDeleted(second));  // a and b both hold key 2
}

TEST_F(ResourceCacheTest, CopyAssignmentFromDefaultRefReleasesCurrent) {
	Ref a = cache.GetOrCreate(1);
	Resource* raw = a.Get();
	Ref empty;

	a = empty;

	EXPECT_FALSE(a.IsValid());
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, CopyAssignmentOntoDefaultRef) {
	Ref a;
	Ref b = cache.GetOrCreate(3);
	a = b;
	EXPECT_EQ(a.Get(), b.Get());
	b = nullptr;
	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(a.Get()));	// a still holds it
}

TEST_F(ResourceCacheTest, CopyAssignmentSelfIsSafe) {
	Ref a = cache.GetOrCreate(1);
	Ref& alias = a;
	a = alias;
	EXPECT_TRUE(a.IsValid());
	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(a.Get()));
}

TEST_F(ResourceCacheTest, CopyAssignmentBetweenRefsToSameEntry) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(1);
	a = b;
	a = nullptr;
	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(b.Get()));
}

// ----------
// Ref Moving
// ----------

TEST_F(ResourceCacheTest, MoveConstructorTransfersOwnershipWithoutChangingCount) {
	Ref a = cache.GetOrCreate(1);
	Resource* raw = a.Get();

	Ref b(std::move(a));

	EXPECT_FALSE(a.IsValid());
	EXPECT_EQ(b.Get(), raw);
	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(raw));

	b = nullptr;
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, MoveAssignmentReleasesOldAndTransfersNew) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(2);
	Resource* first = a.Get();
	Resource* second = b.Get();

	a = std::move(b);

	EXPECT_EQ(a.Get(), second);
	EXPECT_FALSE(b.IsValid());
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(first));
	EXPECT_FALSE(WasDeleted(second));
}

TEST_F(ResourceCacheTest, MoveAssignmentSelfIsSafe) {
	Ref a = cache.GetOrCreate(1);
	Ref& alias = a;
	a = std::move(alias);
	EXPECT_TRUE(a.IsValid());
	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(a.Get()));
}

TEST_F(ResourceCacheTest, MoveAssignmentFromDefaultRef) {
	Ref a = cache.GetOrCreate(1);
	Resource* raw = a.Get();
	a = Ref();
	EXPECT_FALSE(a.IsValid());
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, MoveAssignmentBetweenRefsToSameEntryKeepsCountsBalanced) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(1);
	a = std::move(b);
	a = nullptr;
	b = nullptr;
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(created[0]));
}

TEST_F(ResourceCacheTest, AssignNullptrReleasesReference) {
	Ref a = cache.GetOrCreate(1);
	Resource* raw = a.Get();
	a = nullptr;
	EXPECT_FALSE(a.IsValid());
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, AssignNullptrToEmptyRefIsNoOp) {
	Ref a;
	a = nullptr;
	EXPECT_FALSE(a.IsValid());
}

// ----------
// GetOrCreate
// ----------

TEST_F(ResourceCacheTest, GetOrCreateCallsFactoryWithKey) {
	Ref ref = cache.GetOrCreate(42);
	EXPECT_EQ(factoryCalls, 1);
	EXPECT_EQ(lastKey, 42);
}

TEST_F(ResourceCacheTest, GetOrCreateSameKeyReturnsSameValueAndCreatesOnce) {
	Ref a = cache.GetOrCreate(7);
	Ref b = cache.GetOrCreate(7);
	Ref c = cache.GetOrCreate(7);
	EXPECT_EQ(factoryCalls, 1);
	EXPECT_EQ(a.Get(), b.Get());
	EXPECT_EQ(b.Get(), c.Get());
}

TEST_F(ResourceCacheTest, GetOrCreateDifferentKeysCreateDifferentValues) {
	Ref a = cache.GetOrCreate(1);
	Ref b = cache.GetOrCreate(2);
	EXPECT_EQ(factoryCalls, 2);
	EXPECT_NE(a.Get(), b.Get());
	EXPECT_EQ(a->key, 1);
	EXPECT_EQ(b->key, 2);
}

TEST_F(ResourceCacheTest, GetOrCreateAfterReleaseButBeforeTrimReusesValue) {
	Resource* raw = nullptr;
	{
		Ref a = cache.GetOrCreate(1);
		raw = a.Get();
	}
	Ref b = cache.GetOrCreate(1);
	EXPECT_EQ(factoryCalls, 1);
	EXPECT_EQ(b.Get(), raw);
	EXPECT_FALSE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, GetOrCreateTemporaryPinDoesNotLeakReferences) {
	{
		Ref a = cache.GetOrCreate(1);
	}
	cache.TrimImmediately();
	EXPECT_EQ(DeletedCount(), 1u);
}

TEST_F(ResourceCacheTest, FactoryReturningNullGivesValidRefWithNullValue) {
	cache.SetFactory([](const int&) -> Resource* { return nullptr; });
	Ref ref = cache.GetOrCreate(1);
	EXPECT_TRUE(ref.IsValid());
	EXPECT_EQ(ref.Get(), nullptr);
	ref = nullptr;
	cache.TrimImmediately();
}

TEST_F(ResourceCacheTest, FactoryExceptionPropagatesAndEntryCanBeRetried) {
	bool shouldThrow = true;
	cache.SetFactory([&](const int& key) -> Resource* {
		if (shouldThrow) throw std::runtime_error("load failed");
		Resource* r = new Resource(key);
		std::lock_guard lock(bookkeeping);
		created.push_back(r);
		return r;
	});

	EXPECT_THROW(cache.GetOrCreate(1), std::runtime_error);

	shouldThrow = false;
	Resource* raw = nullptr;
	{
		Ref ref = cache.GetOrCreate(1);
		ASSERT_TRUE(ref.IsValid());
		raw = ref.Get();
		ASSERT_NE(raw, nullptr);
		EXPECT_EQ(raw->key, 1);
	}

	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}

// ----------
// Trim
// ----------

TEST_F(ResourceCacheTest, TrimImmediatelyOnEmptyCacheIsNoOp) {
	cache.TrimImmediately();
	EXPECT_EQ(DeletedCount(), 0u);
}

TEST_F(ResourceCacheTest, TrimImmediatelyDestroysOnlyUnreferencedEntries) {
	Ref keep = cache.GetOrCreate(1);
	Resource* kept = keep.Get();
	Resource* dropped = nullptr;
	{
		Ref temp = cache.GetOrCreate(2);
		dropped = temp.Get();
	}

	cache.TrimImmediately();

	EXPECT_TRUE(WasDeleted(dropped));
	EXPECT_FALSE(WasDeleted(kept));
	EXPECT_EQ(DeletedCount(), 1u);
}

TEST_F(ResourceCacheTest, TrimImmediatelyPassesTheStoredValueToDeleter) {
	Resource* raw = nullptr;
	{
		Ref ref = cache.GetOrCreate(9);
		raw = ref.Get();
	}
	cache.TrimImmediately();
	ASSERT_EQ(DeletedCount(), 1u);
	EXPECT_TRUE(WasDeleted(raw));
}

TEST_F(ResourceCacheTest, TrimImmediatelyTwiceDoesNotDoubleDelete) {
	{
		Ref ref = cache.GetOrCreate(1);
	}
	cache.TrimImmediately();
	cache.TrimImmediately();
	EXPECT_EQ(DeletedCount(), 1u);
}

TEST_F(ResourceCacheTest, EntryIsRecreatedAfterTrim) {
	{
		Ref ref = cache.GetOrCreate(1);
	}
	cache.TrimImmediately();
	Ref again = cache.GetOrCreate(1);
	EXPECT_EQ(factoryCalls, 2);
	EXPECT_TRUE(again.IsValid());
	EXPECT_TRUE(again->alive.load());
}

TEST_F(ResourceCacheTest, TrimImmediatelyHandlesManyEntries) {
	std::vector<Ref> held;
	for (int i = 0; i < 200; ++i) {
		Ref r = cache.GetOrCreate(i);
		if (i % 2 == 0) {
			held.push_back(std::move(r));
		}
	}

	cache.TrimImmediately();
	EXPECT_EQ(DeletedCount(), 100u);
	for (Ref& r : held) {
		EXPECT_TRUE(r->alive.load());
	}
}

TEST_F(ResourceCacheTest, TrimDefersDeletionUntilQueueIsFlushed) {
	Grindstone::DeferredDeletionQueue queue(1, 8);
	Resource* raw = nullptr;
	{
		Ref ref = cache.GetOrCreate(1);
		raw = ref.Get();
	}

	cache.Trim(&queue);

	EXPECT_EQ(DeletedCount(), 0u);
	EXPECT_FALSE(WasDeleted(raw));

	queue.DeleteAll();

	EXPECT_TRUE(WasDeleted(raw));
	EXPECT_EQ(DeletedCount(), 1u);
}

TEST_F(ResourceCacheTest, TrimRemovesEntryFromCacheImmediately) {
	Grindstone::DeferredDeletionQueue queue(1, 8);
	{
		Ref ref = cache.GetOrCreate(1);
	}
	cache.Trim(&queue);

	Ref again = cache.GetOrCreate(1);
	EXPECT_EQ(factoryCalls, 2);
	EXPECT_NE(again.Get(), created[0]);
	queue.DeleteAll();
	EXPECT_TRUE(again->alive.load());
}

TEST_F(ResourceCacheTest, TrimKeepsReferencedEntries) {
	Grindstone::DeferredDeletionQueue queue(1, 8);
	Ref keep = cache.GetOrCreate(1);
	cache.Trim(&queue);
	queue.DeleteAll();
	EXPECT_EQ(DeletedCount(), 0u);
	EXPECT_TRUE(keep->alive.load());
}

// ----------
// Threading
// ----------

TEST_F(ResourceCacheTest, ConcurrentGetOrCreateSameKeyRunsFactoryOnce) {
	constexpr int kThreads = 8;
	constexpr int kIterations = 2000;
	std::atomic<bool> go{false};
	std::vector<Resource*> seen(kThreads, nullptr);
	std::atomic<int> mismatches{0};

	std::vector<std::thread> threads;
	for (int t = 0; t < kThreads; ++t) {
		threads.emplace_back([&, t] {
			while (!go.load()) std::this_thread::yield();
			for (int i = 0; i < kIterations; ++i) {
				Ref r = cache.GetOrCreate(1);
				if (seen[t] == nullptr) {
					seen[t] = r.Get();
				} else if (seen[t] != r.Get()) {
					++mismatches;
				}
			}
		});
	}
	go = true;
	for (std::thread& th : threads) {
		th.join();
	}

	EXPECT_EQ(factoryCalls, 1);
	EXPECT_EQ(mismatches.load(), 0);
	for (int t = 1; t < kThreads; ++t) {
		EXPECT_EQ(seen[t], seen[0]);
	}
}

TEST_F(ResourceCacheTest, ConcurrentGetOrCreateDistinctKeys) {
	constexpr int kThreads = 8;
	constexpr int kKeysPerThread = 100;
	std::vector<std::thread> threads;
	std::vector<std::vector<Ref>> held(kThreads);

	for (int t = 0; t < kThreads; ++t) {
		threads.emplace_back([&, t] {
			for (int i = 0; i < kKeysPerThread; ++i) {
				held[t].push_back(cache.GetOrCreate(t * kKeysPerThread + i));
			}
		});
	}
	for (std::thread& th : threads) {
		th.join();
	}

	EXPECT_EQ(factoryCalls, kThreads * kKeysPerThread);
	for (int t = 0; t < kThreads; ++t) {
		for (int i = 0; i < kKeysPerThread; ++i) {
			EXPECT_EQ(held[t][i]->key, t * kKeysPerThread + i);
		}
	}
}

TEST_F(ResourceCacheTest, HeldRefsNeverObserveDeadResourceWhileTrimming) {
	constexpr int kWorkers = 6;
	constexpr int kIterations = 5000;
	constexpr int kKeys = 4;
	std::atomic<bool> stop{false};
	std::atomic<int> deadSeen{0};

	std::thread trimmer([&] {
		while (!stop.load()) cache.TrimImmediately();
	});

	std::vector<std::thread> workers;
	for (int t = 0; t < kWorkers; ++t) {
		workers.emplace_back([&, t] {
			for (int i = 0; i < kIterations; ++i) {
				Ref a = cache.GetOrCreate((i + t) % kKeys);
				Ref b = a;	// copy while trimmer is running
				if (!a->alive.load() || !b->alive.load()) {
					++deadSeen;
				}

				b = nullptr;
				if (!a->alive.load()) {
					++deadSeen;
				}
			}
		});
	}
	for (std::thread& worker : workers) {
		worker.join();
	}
	stop = true;
	trimmer.join();

	EXPECT_EQ(deadSeen.load(), 0);
}

TEST_F(ResourceCacheTest, ConcurrentRefCopyAndReleaseKeepsCountBalanced) {
	Ref shared = cache.GetOrCreate(1);
	Resource* raw = shared.Get();

	std::vector<std::thread> threads;
	for (int t = 0; t < 8; ++t) {
		threads.emplace_back([&] {
			for (int i = 0; i < 20000; ++i) {
				Ref copy(shared);
				Ref moved(std::move(copy));
				moved = nullptr;
			}
		});
	}
	for (std::thread& worker : threads) {
		worker.join();
	}

	cache.TrimImmediately();
	EXPECT_FALSE(WasDeleted(raw));
	shared = nullptr;
	cache.TrimImmediately();
	EXPECT_TRUE(WasDeleted(raw));
}
}  // namespace Grindstone::Common::Containers
