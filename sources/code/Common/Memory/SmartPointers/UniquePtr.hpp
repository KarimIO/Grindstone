#pragma once

template<typename T>
class UniquePtr {
public:

	UniquePtr() = default;

	UniquePtr(T* ptr) : ptr(ptr) {}

	template<typename... Args>
	UniquePtr(Args&&... params) : ptr(new T(std::forward<Args>(params)...)) {}

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
			delete ptr;
		}
	}

private:
	T* ptr = nullptr;
};

template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... params) {
	return new T(std::forward<Args>(params)...);
}
