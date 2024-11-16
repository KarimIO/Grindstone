#pragma once

#include <functional>

namespace Grindstone::Memory::SmartPointers {
	template<typename T>
	class UniquePtr {
	public:

		UniquePtr() = default;

		explicit UniquePtr(T* ptr, std::function<void(void*)> deleteFn) : ptr(ptr), deleteFn(deleteFn) {}
		UniquePtr(std::nullptr_t) : ptr(nullptr), deleteFn(nullptr) {}

		UniquePtr(const UniquePtr<T>& other) = delete;
		UniquePtr& operator=(const UniquePtr<T>& other) = delete;

		UniquePtr& operator=(std::nullptr_t) {
			Release();
			return *this;
		}

		UniquePtr(UniquePtr<T>&& other) noexcept : ptr(nullptr), deleteFn(nullptr) {
			std::swap(other.ptr, ptr);
			std::swap(other.deleteFn, deleteFn);
		}

		UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept {
			if (other.ptr != nullptr) {
				Release();
			}

			ptr = other.ptr;
			deleteFn = other.deleteFn;

			other.Release();

			return *this;
		}

		template<typename U>
		UniquePtr(UniquePtr<U>&& other) {
			UniquePtr<T> tmp(other.Release());
			tmp.swap(*this);
		}

		template<typename U>
		UniquePtr& operator=(UniquePtr<U>&& other) {
			auto deleteFn = other.GetDeleter();
			UniquePtr<T> tmp(other.Release(), deleteFn);
			tmp.Swap(*this);
			return *this;
		}

		T* Release() noexcept {
			T* emptyPtr = nullptr;
			std::swap(emptyPtr, ptr);
			deleteFn = nullptr;

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
		}

		void Swap(UniquePtr& other) noexcept {
			std::swap(ptr, other.ptr);
			std::swap(deleteFn, other.deleteFn);
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

		std::function<void(void*)> GetDeleter() const {
			return this->deleteFn;
		}

		~UniquePtr() {
			Reset();
		}

	private:
		T* ptr = nullptr;
		std::function<void(void*)> deleteFn;
	};

	template<typename T, typename... Args>
	UniquePtr<T> MakeUnique(Args&&... params) {
		return new T(std::forward<Args>(params)...);
	}
}
