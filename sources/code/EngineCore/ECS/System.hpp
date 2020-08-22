#pragma once

#include <cinttypes>

namespace Grindstone {
	class Scene;

	namespace ECS {
		using SystemType = std::uint8_t;
		const SystemType MAX_SYSTEMS = UINT8_MAX;
		const SystemType INVALID_SYSTEM = UINT8_MAX;

		class ISystem {
		public:
			virtual void update() = 0;

			Scene *scene_;
			SystemType system_type_;
		};
	}
}

#define SYSTEM_BODY(T)	static Grindstone::ECS::SystemType static_system_type_; \
						static const char *system_name_; \
						T(Grindstone::Scene *s) { scene_ = s; system_type_ = Grindstone::ECS::INVALID_SYSTEM; } \
						static Grindstone::ECS::ISystem* createSystem(Grindstone::Scene *s) { return new T(s); }
#define SYSTEM_DEFINE(T, Name) Grindstone::ECS::SystemType T::static_system_type_ = 0; const char * T::system_name_ = Name