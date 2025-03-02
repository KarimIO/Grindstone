#include "Uuid.hpp"
#include <Common/Assert.hpp>

#if defined(_WIN32)
#include <Rpc.h>

Grindstone::Uuid Grindstone::Uuid::CreateRandom() {
	Uuid newUuid;
	bool hasSuccessfullyCreatedRandomUuid = UuidCreate((::UUID*)&(newUuid.uuid)) == RPC_S_OK;
	GS_ASSERT_ENGINE_WITH_MESSAGE(hasSuccessfullyCreatedRandomUuid, "Could not create a random uuid.")

	return newUuid;
}

bool Grindstone::Uuid::MakeFromString(const char* str, Grindstone::Uuid& outUuid) {
	if (str == nullptr || strlen(str) == 0) {
		memset(outUuid.uuid, 0, sizeof(outUuid.uuid));
		return false;
	}

	if (UuidFromString((unsigned char*)str, (::UUID*)&outUuid.uuid) != RPC_S_OK) {
		GS_BREAK_WITH_MESSAGE("Could not make guid from an empty string.")
			memset(outUuid.uuid, 0, sizeof(outUuid.uuid));
		return false;
	}

	return true;
}

std::string Grindstone::Uuid::ToString() const {
	unsigned char* uuidCstr;
	if (UuidToString((::UUID*)&uuid, &uuidCstr) == RPC_S_OK) {
		std::string uuidStr((char*)uuidCstr);
		RpcStringFreeA(&uuidCstr);

		return uuidStr;
	}

	return std::string();
}
#else
#include <uuid/uuid.h>

Grindstone::Uuid Grindstone::Uuid::CreateRandom() {
	Uuid newUuid();
	uuid_generate_random((uuid_t)newUuid.uuid);
	return newUuid;
}

bool Grindstone::Uuid::MakeFromString(const char* str, Grindstone::Uuid& outUuid) {
	if (str == nullptr || strlen(str) == 0) {
		memset(outUuid.uuid, 0, sizeof(outUuid.uuid));
		return false;
	}

	return uuid_parse(str, (uuint_t)outUuid.uuid) == 0;
}

std::string Grindstone::Uuid::ToString() const {
	char uuidStr[37];
	uuid_unparse((uuid_t)uuid, uuidStr);

	return uuidStr;
}
#endif

Grindstone::Uuid::Uuid() {
	memset(uuid, 0, sizeof(uuid));
}

bool Grindstone::Uuid::MakeFromString(const std::string& str, Grindstone::Uuid& outUuid) {
	return MakeFromString(str.c_str(), outUuid);
}

bool Grindstone::Uuid::IsValid() const {
	const uint64_t* uuidCasted = reinterpret_cast<const uint64_t*>(&uuid[0]);
	return uuidCasted[0] != 0 || uuidCasted[1] != 0;
}

Grindstone::Uuid::operator std::string() const {
	return ToString();
}

bool Grindstone::Uuid::operator==(const Uuid& other) const {
	return std::memcmp(other.uuid, uuid, sizeof(uuid)) == 0;
}

bool Grindstone::Uuid::operator!=(const Uuid& other) const {
	return std::memcmp(other.uuid, uuid, sizeof(uuid)) != 0;
}

bool Grindstone::Uuid::operator<(const Uuid& other) const {
	return std::memcmp(other.uuid, uuid, sizeof(uuid)) < 0;
}

void Grindstone::Uuid::operator=(const Uuid& other) {
	std::memcpy(uuid, other.uuid, sizeof(uuid));
}
