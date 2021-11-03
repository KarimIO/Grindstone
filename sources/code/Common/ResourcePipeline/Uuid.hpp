#pragma once

#ifdef _WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif

namespace Grindstone {
	class Uuid {
	public:
#ifdef _WIN32
		Uuid() {
			UuidCreate( &uuid );
		}

		Uuid(std::string str) {
			UuidFromString((unsigned char*)str.c_str(), &uuid);
		}

		std::string ToString() {
			unsigned char* uuidCstr;
			UuidToString( &uuid, &uuidCstr );
			std::string uuidStr( (char*) uuidCstr );
			RpcStringFreeA ( &uuidCstr );

			return uuidStr;
		}
#else
		Uuid CreateNew() {
			uuid_t uuid;
			uuid_generate_random ( uuid );
			return uuid;
		}

		std::string ToString(Uuid uuid) {
			char uuidStr[37];
			uuid_unparse(uuid, uuidStr);

			return uuidStr;
		}

		Uuid FromString(std::string str) {
			uuint_t uuid;
			uuid_parse(str.c_str(), uuid);
			return uuid;
		}
#endif

	operator std::string() {
		return ToString();
	}

	private:
#ifdef _WIN32
		UUID uuid;
#else
		uuid_t uuid;
#endif
	};
}
