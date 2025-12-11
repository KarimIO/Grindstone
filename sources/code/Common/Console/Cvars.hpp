#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

#include "../HashedString.hpp"

namespace Grindstone {
	enum class CvarType : uint8_t {
		Undefined = 0,
		Integer,
		Float,
		String,
	};

	enum class CvarFlags : uint16_t {
		None = 0,
		Hidden = 1 << 1,						// Does not show in console and cvar UI
		ReadOnlyUi = 1 << 2,					// Is read-only in the console and cvar UI.
		ReadOnlyCode = 1 << 3,					// Is read-only in the code.
		ReadOnly = ReadOnlyUi | ReadOnlyCode,	// Completely read-only.
		NotSerialized = 1 << 4,					// Cannot be saved/loaded to a file.
		Advanced = 1 << 5,						// Considered advanced functionality.
		NotCommandLineParameter = 1 << 6,		// Cannot be passed to the application as a CLI parameter (ie: editor.exe -parameterName=4)
		Boolean = 1 << 7,						// This int is represented as a boolean.
		EditorNumberSlider = 1 << 8,			// Used for Integers and Floats, uses a slider in Cvar UI.
	};

	class CvarParameter {
	public:
		friend class CvarSystemImpl;

		int32_t arrayIndex;

		CvarType type = CvarType::Undefined;
		CvarFlags flags = CvarFlags::None;
		std::string name;
		std::string description;
	};

	class CvarSystem {
	public:
		static CvarSystem* GetInstance();
		static void SetInstance(CvarSystem* ptr);
		virtual CvarParameter* GetCvar(Grindstone::HashedString name) = 0;
		virtual ~CvarSystem() {}

		virtual size_t GetFloatCount() const = 0;
		virtual size_t GetIntCount() const = 0;
		virtual size_t GetStringCount() const = 0;

		virtual double* GetFloatCvar(Grindstone::HashedString name) = 0;
		virtual int32_t* GetIntCvar(Grindstone::HashedString name) = 0;
		virtual const char* GetStringCvarCstring(Grindstone::HashedString name) = 0;
		virtual std::string* GetStringCvar(Grindstone::HashedString name) = 0;

		virtual void SetFloatCvar(Grindstone::HashedString name, double value) = 0;
		virtual void SetIntCvar(Grindstone::HashedString name, int32_t value) = 0;
		virtual void SetStringCvar(Grindstone::HashedString name, const char* value) = 0;

		virtual double* GetFloatCvar(size_t arrayIndex) = 0;
		virtual int32_t* GetIntCvar(size_t arrayIndex) = 0;
		virtual const char* GetStringCvarCstring(size_t arrayIndex) = 0;
		virtual std::string* GetStringCvar(size_t arrayIndex) = 0;

		virtual void SetFloatCvar(size_t arrayIndex, double value) = 0;
		virtual void SetIntCvar(size_t arrayIndex, int32_t value) = 0;
		virtual void SetStringCvar(size_t arrayIndex, const char* value) = 0;

		virtual CvarParameter* CreateFloatCvar(const char* name, const char* description, double defaultValue, double currentValue, CvarFlags flags = CvarFlags::None) = 0;
		virtual CvarParameter* CreateIntCvar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue, CvarFlags flags = CvarFlags::None) = 0;
		virtual CvarParameter* CreateBooleanCvar(const char* name, const char* description, bool defaultValue, bool currentValue, CvarFlags flags = CvarFlags::None) = 0;
		virtual CvarParameter* CreateStringCvar(const char* name, const char* description, const char* defaultValue, const char* currentValue, CvarFlags flags = CvarFlags::None) = 0;
		
		using Iterator = std::unordered_map<Grindstone::HashValue, CvarParameter>::iterator;
		using ConstIterator = std::unordered_map<Grindstone::HashValue, CvarParameter>::const_iterator;

		virtual Iterator begin() noexcept = 0;
		virtual Iterator end() noexcept = 0;

		virtual ConstIterator begin() const noexcept = 0;
		virtual ConstIterator end() const noexcept = 0;
	};

	CvarSystem* CreateCvarSystemInstance();
}
