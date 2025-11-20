#include <mutex>
#include <functional>
#include <unordered_map>

namespace Grindstone {
	using ObserverHandle = size_t;

	template<typename... Args>
	class SinglecastObservable {
	public:
		using ObserverFunction = std::function<void(Args...)>;

		void Broadcast(Args... args) {
			std::scoped_lock lck(mutex);
			function(args...);
		}

		void Subscribe(ObserverFunction func) {
			std::scoped_lock lck(mutex);
			function = func;
		}

		void Unsubscribe() {
			std::scoped_lock lck(mutex);
			function = nullptr;
		}

	protected:

		std::mutex mutex;
		ObserverFunction function;

	};

	template<typename... Args>
	class MulticastObservable {
	public:
		using ObserverFunction = std::function<void(Args...)>;

		void Broadcast(Args... args) {
			std::scoped_lock lck(mutex);
			for (auto& [_, func] : observers) {
				func(args...);
			}
		}

		ObserverHandle Subscribe(ObserverFunction func) {
			std::scoped_lock lck(mutex);
			ObserverHandle handle = ++currentHandle;
			observers.emplace(handle, func);
			return handle;
		}

		void Unsubscribe(ObserverHandle handle) {
			std::scoped_lock lck(mutex);
			observers.erase(handle);
		}

	protected:

		std::mutex mutex;
		ObserverHandle currentHandle = 0;
		std::unordered_map<ObserverHandle, ObserverFunction> observers;

	};
}
