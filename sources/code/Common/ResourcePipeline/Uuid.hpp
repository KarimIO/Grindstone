#pragma once

#include <string>

namespace Grindstone {
	class Uuid {
	public:
		static bool MakeFromString(const std::string& str, Grindstone::Uuid& outUuid);
		static bool MakeFromString(const char* str, Grindstone::Uuid& outUuid);

		Uuid();
		void operator=(const Uuid& other);
		std::string ToString() const;
		bool operator==(const Uuid& other) const;
		bool operator!=(const Uuid& other) const;
		bool operator<(const Uuid& other) const;
		bool IsValid() const;
		operator std::string() const;

		static Uuid CreateRandom();

	protected:
		char uuid[16];
	};
}
