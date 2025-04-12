#pragma once

#include <stdint.h>
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

		union {
			uint8_t asUint8[16];
			uint16_t asUint16[8];
			uint32_t asUint32[4];
			uint64_t asUint64[2];
		};
	};
}

template <>
struct std::hash<Grindstone::Uuid>
{
	std::size_t operator()(const Grindstone::Uuid& k) const {
		using std::size_t;

		return k.asUint64[0] ^ k.asUint64[1];
	}
};
