#pragma once

#include <functional>

template<typename T>
class UniquePtr {
public:

	UniquePtr() = default;

	UniquePtr(T* ptr, std::function<void(void*)> deleteFn) : ptr(ptr), deleteFn(deleteFn) {}

	UniquePtr(const UniquePtr& obj) = delete;
	UniquePtr& operator=(const UniquePtr& obj) = delete;

	T* operator->() {
		return this->ptr;
	}

	T& operator*() {
		return *(this->ptr);
	}

	~UniquePtr() {
		if (ptr != nullptr) {
			ptr->~T();

			if (deleteFn) {
				deleteFn(ptr);
			}

			ptr = nullptr;
		}
	}

private:
	T* ptr = nullptr;
	std::function<void(void*)> deleteFn;
};

template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... params) {
	return new T(std::forward<Args>(params)...);
}
