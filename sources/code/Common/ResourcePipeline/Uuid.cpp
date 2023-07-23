#include "Uuid.hpp"

#if defined(_WIN32)
#include <Rpc.h>

Grindstone::Uuid Grindstone::Uuid::CreateRandom() {
	Uuid newUuid;
	UuidCreate((UUID*)&(newUuid.uuid));

	return newUuid;
}

Grindstone::Uuid::Uuid(const char* str) {
	UuidFromString((unsigned char*)str, (UUID*)&uuid);
}

std::string Grindstone::Uuid::ToString() {
	unsigned char* uuidCstr;
	UuidToString((UUID*)&uuid, &uuidCstr);
	std::string uuidStr( (char*) uuidCstr );
	RpcStringFreeA(&uuidCstr);

	return uuidStr;
}
#else
#include <uuid/uuid.h>

Grindstone::Uuid Grindstone::Uuid::CreateRandom() {
	Uuid newUuid();
	uuid_generate_random((uuid_t)newUuid.uuid);
	return newUuid;
}

Grindstone::Uuid::Uuid(const char* str) {
	uuid_parse(str, (uuint_t)uuid);
}

std::string Grindstone::Uuid::ToString() {
	char uuidStr[37];
	uuid_unparse((uuid_t)uuid, uuidStr);

	return uuidStr;
}
#endif

Grindstone::Uuid::Uuid() {
	memset(uuid, 0, sizeof(uuid));
}

Grindstone::Uuid::Uuid(std::string str) : Uuid(str.c_str()) {}

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
	*this = Uuid(str);
}

void Grindstone::Uuid::operator=(const Uuid& other) {
	std::memcpy(uuid, other.uuid, sizeof(uuid));
}
