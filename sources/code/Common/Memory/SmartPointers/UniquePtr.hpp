#pragma once

#include <functional>

namespace Grindstone {
	template<typename T>
	class UniquePtr {
	public:

		UniquePtr() : ptr(nullptr), deleteFn() {}

		explicit UniquePtr(T* ptr, std::function<void(void*)> newDeleteFn) : ptr(ptr), deleteFn(newDeleteFn) {}

		UniquePtr(std::nullptr_t) : ptr(nullptr), deleteFn() {}

		// Deleted Copy operations
		UniquePtr(const UniquePtr<T>& other) = delete;
		UniquePtr& operator=(const UniquePtr<T>& other) = delete;

		UniquePtr& operator=(std::nullptr_t) {
			Release();
			return *this;
		}

		UniquePtr(UniquePtr&& other) noexcept
			: ptr(other.ptr), deleteFn(std::move(other.deleteFn)) {
			other.ptr = nullptr;
			other.deleteFn = std::function<void(void*)>{};

		}

		UniquePtr& operator=(UniquePtr&& other) noexcept {
			UniquePtr tmp(std::move(other));
			Swap(tmp);
			return *this;
		}

		template<typename U,
			std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		UniquePtr(UniquePtr<U>&& other) noexcept
			: ptr(other.Release()), deleteFn(std::move(other.GetDeleter())) {}

		template<typename U,
			std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		UniquePtr& operator=(UniquePtr<U>&& other) noexcept {
			if (this != reinterpret_cast<UniquePtr<T>*>(&other)) {
				Reset();
				ptr = other.Release();
				deleteFn = std::move(other.GetDeleter());
			}
			return *this;
		}

		T* Release() noexcept {
			T* emptyPtr = nullptr;
			std::swap(emptyPtr, ptr);
			deleteFn = std::function<void(void*)>{};

			return emptyPtr;
		}

		void Reset() noexcept {
			if (ptr != nullptr) {
				ptr->~T();

				if (deleteFn) {
					deleteFn(ptr);
				}

				ptr = nullptr;
			}

			deleteFn = std::function<void(void*)>{};
		}

		void Swap(UniquePtr& other) noexcept {
			std::swap(ptr, other.ptr);
			std::swap(deleteFn, other.deleteFn);
		}

		bool operator==(const Grindstone::UniquePtr<T>& other) const noexcept {
			return ptr == other.ptr && deleteFn == other.deleteFn;
		}

		bool operator==(const T* other) const noexcept {
			return ptr == other;
		}

		bool operator==(std::nullptr_t) const noexcept {
			return ptr == nullptr;
		}

		explicit operator T*() const {
			return ptr;
		}

		explicit operator bool() const {
			return ptr != nullptr;
		}

		T* operator->() const {
			return this->ptr;
		}

		T& operator*() const {
			return *(this->ptr);
		}

		T* Get() const {
			return this->ptr;
		}

		const std::function<void(void*)>& GetDeleter() const {
			return this->deleteFn;
		}

		~UniquePtr() {
			Reset();
		}

		template<typename U>
		bool operator==(const UniquePtr<U>& other) const noexcept {
			static_assert(std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>,
				"UniquePtr comparison requires compatible pointer types.");
			return ptr == other.ptr && deleteFn == other.deleteFn;
		}

	private:
		T* ptr = nullptr;
		std::function<void(void*)> deleteFn;
	};
}
