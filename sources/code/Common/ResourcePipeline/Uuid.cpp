#include "Uuid.hpp"

#if defined(_WIN32)
#include <Rpc.h>

Grindstone::Uuid::Uuid() {
	UuidCreate( (UUID*)&uuid );
}

void Grindstone::Uuid::FromString(std::string str) {
	UuidFromString((unsigned char*)str.c_str(), (UUID*)&uuid);
}

std::string Grindstone::Uuid::ToString() {
	unsigned char* uuidCstr;
	UuidToString((UUID*)&uuid, &uuidCstr);
	std::string uuidStr( (char*) uuidCstr );
	RpcStringFreeA ( &uuidCstr );

	return uuidStr;
}
#else
#include <uuid/uuid.h>

Grindstone::Uuid::Uuid() {
	uuid_generate_random((uuid_t)uuid);
}

void Grindstone::Uuid::FromString(std::string str) {
	uuint_t uuid;
	uuid_parse(str.c_str(), uuid);
}

std::string Grindstone::Uuid::ToString() {
	char uuidStr[37];
	uuid_unparse((uuid_t)uuid, uuidStr);

	return uuidStr;
}
#endif

Grindstone::Uuid::Uuid(std::string str) {
	FromString(str);
}

Grindstone::Uuid::operator std::string() {
	return ToString();
}

bool Grindstone::Uuid::operator==(const Uuid& other) const {
	return std::memcmp(other.uuid, uuid, sizeof(uuid)) == 0;
}

bool Grindstone::Uuid::operator<(const Uuid& other) const {
	return std::memcmp(other.uuid, uuid, sizeof(uuid)) < 0;
}

void Grindstone::Uuid::operator=(const char *str) {
	FromString(str);
}
