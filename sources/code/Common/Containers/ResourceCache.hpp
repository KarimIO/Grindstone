#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <Common/Assert.hpp>
#include <Common/DeferredDeletionQueue.hpp>

namespace Grindstone::Containers {
	template<typename Key, typename Value, class Hash = std::hash<Key>>
	class ResourceCache {
	private:
		struct Entry {
			Key key;
			Value value;
			std::atomic<uint32_t> refs{0};
			std::once_flag created;
		};

	public:
		class Ref {
			friend class ResourceCache;

			Entry* entry = nullptr;

		public:
			Ref() = default;
			explicit Ref(Entry* entry) : entry(entry) {
				if (entry) {
					entry->refs.fetch_add(1, std::memory_order_relaxed);
				}
			}
			explicit Ref(std::nullptr_t) : entry(nullptr) {}
			Ref(const Ref& other) : Ref(other.entry) {}
			Ref(Ref&& other) noexcept : entry(std::exchange(other.entry, nullptr)) {}

			Ref& operator=(std::nullptr_t other) {
				if (entry != nullptr) {
					entry->refs.fetch_sub(1, std::memory_order_acq_rel);
					entry = nullptr;
				}

				return *this;
			}

			Ref& operator=(const Ref& other) {
				if (other.entry != entry) {
					if (other.entry != nullptr) {
						other.entry->refs.fetch_add(1, std::memory_order_relaxed);
					}

					if (entry != nullptr) {
						entry->refs.fetch_sub(1, std::memory_order_acq_rel);
					}

					entry = other.entry;
				}

				return *this;
			}

			Ref& operator=(Ref&& other) noexcept {
				if (other.entry != entry) {
					if (entry != nullptr) {
						entry->refs.fetch_sub(1, std::memory_order_acq_rel);
					}

					entry = std::exchange(other.entry, nullptr);
				}

				return *this;
			}

			bool operator==(const Ref& other) const {
				return entry == other.entry;
			}

			bool operator!=(const Ref& other) const {
				return entry != other.entry;
			}

			bool IsValid() const {
				return entry != nullptr;
			}

			~Ref() {
				if (entry) {
					entry->refs.fetch_sub(1, std::memory_order_acq_rel);
				}
			}
			Value operator->() const {
				if (entry) {
					return entry->value;
				}

				return nullptr;
			}

			Value Get() const {
				if (entry) {
					return entry->value;
				}

				return nullptr;
			}
		};

		using Factory = std::function<Value(const Key&)>;
		using Deleter = std::function<void(Value)>;

		~ResourceCache() {
			TrimImmediately();
			GS_ASSERT_WITH_MESSAGE(cache.empty(), "ResourceCache handles outlive cache.");
		}

		// If key exists in the cache, return it, if not, create it via factory.
		Ref GetOrCreate(const Key& key) {
			Ref ref;
			{
				std::unique_lock lock(mutex);
				auto [it, inserted] = cache.try_emplace(key);
				Entry* entry = &it->second;
				if (inserted) {
					entry->key = key;
				}
				ref = Ref(entry);
			}

			std::call_once(ref.entry->created, [&]{ ref.entry->value = constructCallback(key); });

			return ref;
		}

		// Remove all entries that are unused (where ref count==0).
		void Trim(Grindstone::DeferredDeletionQueue* deferredDeletionQueue) {
			std::unique_lock lock(mutex);
			for (auto it = cache.begin(); it != cache.end();) {
				if (it->second.refs.load() == 0) {
					deferredDeletionQueue->PushDeletion(
						[resource=it->second.value, destroyCallback=this->destroyCallback]() {
							destroyCallback(resource);
						}
					);
					it = cache.erase(it);
				} else {
					++it;
				}
			}
		}
		
		// Remove all entries that are unused (where ref count==0).
		void TrimImmediately() {
			std::unique_lock lock(mutex);
			for (auto it = cache.begin(); it != cache.end();) {
				if (it->second.refs.load() == 0) {
					destroyCallback(it->second.value);
					it = cache.erase(it);
				} else {
					++it;
				}
			}
		}

		void SetFactory(Factory factory) { constructCallback = factory; }
		void SetDeleter(Deleter deleter) { destroyCallback = deleter; }

	protected:

		Factory constructCallback;
		Deleter destroyCallback;

		std::mutex mutex;
		std::unordered_map<Key, Entry, Hash> cache;
	};
}
