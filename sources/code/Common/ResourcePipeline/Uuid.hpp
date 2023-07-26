#pragma once

#include <string>

namespace Grindstone {
	class Uuid {
	public:
		Uuid();
		Uuid(std::string str);
		Uuid(const char* str);
		std::string ToString();
		void operator=(const char*);
		void operator=(const Uuid& other);
		bool operator==(const Uuid& other) const;
		bool operator!=(const Uuid& other) const;
		bool operator<(const Uuid& other) const;
		bool IsValid();
		operator std::string();

		static Uuid CreateRandom();

	protected:
		char uuid[16];
	};
}
