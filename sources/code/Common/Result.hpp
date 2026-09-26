#pragma once

#include <variant>

namespace Grindstone {
	template<typename Value, typename Error>
	struct Result {
		static_assert(std::is_copy_constructible_v<Value>);
		static_assert(std::is_copy_assignable_v<Value>);
		static_assert(std::is_trivially_copyable_v<Error>);

		Result() = default;
		Result(Value val) : storage(val) {}
		Result(Error err) : storage(err) {}

		Value GetValue() const {
			return std::get<Value>(storage);
		}

		Error GetError() const {
			return std::get<Error>(storage);
		}

		bool HasValue() const noexcept {
			return std::holds_alternative<Value>(storage);
		}

		bool HasError() const noexcept {
			return std::holds_alternative<Error>(storage);
		}

		operator bool() const noexcept {
			return std::holds_alternative<Value>(storage);
		}

		std::variant<Value, Error> storage;
	};
}
