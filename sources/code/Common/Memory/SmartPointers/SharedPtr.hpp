#pragma once

struct SharedPtrRefCounter {
	unsigned int refCount = 1;
};

template<typename T>
class SharedPtr {
public:

	SharedPtr() = default;

	SharedPtr(T* ptr) : ptr(ptr), refCounter(new SharedPtrRefCounter()) {}

	SharedPtr(const SharedPtr& obj) : ptr(obj.ptr), refCounter(obj.refCounter) {
		if (refCounter) {
			++refCounter->refCount;	
		}
	}

	SharedPtr& operator=(const SharedPtr& obj) {
		if (ptr && refCounter) {
			if (--refCounter->refCount == 0) {
				delete ptr;
				delete refCounter;
			}
		}

		ptr = obj->ptr;
		refCounter = obj->refCounter;

		if (ptr && refCounter) {
			++refCounter->refCount;
		}
	}

	T* operator->() {
		return this->ptr;
	}

	T& operator*() {
		return *(this->ptr);
	}

	~SharedPtr() {
		if (refCounter != nullptr && ptr != nullptr) {
			if (--refCounter->refCount == 0) {
				delete ptr;
				delete refCounter;
			}
		}
	}

private:
	T* ptr = nullptr;
	SharedPtrRefCounter* refCounter = nullptr;
};

template<typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... params) {
	return new T(std::forward<Args>(params)...);
}
