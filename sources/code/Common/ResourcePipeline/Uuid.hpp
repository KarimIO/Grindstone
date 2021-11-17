#pragma once

#include <string>

namespace Grindstone {
	class Uuid {
	public:
		Uuid();
		Uuid(std::string str);
		void FromString(std::string str);
		std::string ToString();
		void operator=(const char*);
		bool operator==(const Uuid& other) const;
		bool operator<(const Uuid& other) const;
		operator std::string();

	private:
		char uuid[16];
	};
}
