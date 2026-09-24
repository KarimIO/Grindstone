#pragma once

#include <functional>

namespace Grindstone {
	template<typename T>
	class SharedPtr {
	public:
		struct SharedPtrRefCounter {
			unsigned int refCount = 1;
		};

		SharedPtr() = default;

		SharedPtr(T* ptr, std::function<void(void*)> deleteFn)
			: ptr(ptr),
			  refCounter(new SharedPtrRefCounter()),
			  deleteFn(deleteFn) {}

		SharedPtr(const SharedPtr& obj)
			: ptr(obj.ptr), refCounter(obj.refCounter), deleteFn(obj.deleteFn) {
			if (refCounter) {
				++refCounter->refCount;
			}
		}

		SharedPtr& operator=(const SharedPtr& obj) {
			if (ptr && refCounter && --refCounter->refCount == 0) {
				ptr->~T();
				deleteFn(ptr);
				delete refCounter;
			}

			ptr = obj.ptr;
			deleteFn = obj.deleteFn;
			refCounter = obj.refCounter;

			if (ptr && refCounter) {
				++refCounter->refCount;
			}

			return *this;
		}

		T* operator->() {
			return this->ptr;
		}

		T& operator*() {
			return *(this->ptr);
		}

		const T* operator->() const {
			return this->ptr;
		}

		const T& operator*() const {
			return *(this->ptr);
		}

		~SharedPtr() {
			if (refCounter != nullptr &&
				ptr != nullptr &&
				--refCounter->refCount == 0
			) {
				ptr->~T();
				deleteFn(ptr);
				delete refCounter;
			}
		}

	private:
		T* ptr = nullptr;
		SharedPtrRefCounter* refCounter = nullptr;
		std::function<void(void*)> deleteFn;
	};

	template<typename T, typename... Args>
	SharedPtr<T> MakeShared(Args&&... params) {
		return new T(std::forward<Args>(params)...);
	}
}
