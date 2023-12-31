#pragma once

#include <string>

namespace Grindstone {
	class Uuid {
	public:
		Uuid();
		Uuid(std::string str);
		Uuid(const char* str);
		void operator=(const char*);
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
