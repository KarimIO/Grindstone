#include <unordered_map>

#include <array>
#include <algorithm>
#include <shared_mutex>

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "Cvars.hpp"

using namespace Grindstone;

static Grindstone::CvarSystem* cvarSystem = nullptr;

namespace Grindstone {
	template<typename T>
	struct CvarStorage {
		T initial;
		T current;
		CvarParameter* parameter;
	};

	template<typename T>
	struct CvarArray {
		CvarStorage<T>* cvars{ nullptr };
		size_t count{ 0 };

		CvarArray(size_t size) {
			cvars = Grindstone::Memory::AllocatorCore::AllocateArray<CvarStorage<T>>(size);
		}

		CvarStorage<T>* GetCurrentStorage(int32_t index) {
			return &cvars[index];
		}

		T* GetCurrentPtr(int32_t index) {
			return &cvars[index].current;
		};

		T GetCurrent(int32_t index) {
			return cvars[index].current;
		};

		void SetCurrent(const T& val, int32_t index) {
			cvars[index].current = val;
		}

		int Add(const T& value, CvarParameter* param) {
			size_t index = count;

			cvars[index].current = value;
			cvars[index].initial = value;
			cvars[index].parameter = param;

			param->arrayIndex = index;
			count++;
			return index;
		}

		int Add(const T& initialValue, const T& currentValue, CvarParameter* param) {
			size_t index = count;

			cvars[index].current = currentValue;
			cvars[index].initial = initialValue;
			cvars[index].parameter = param;

			param->arrayIndex = index;
			count++;

			return index;
		}
	};

	class CvarSystemImpl : public CvarSystem {
	public:
		CvarParameter* GetCvar(Grindstone::HashedString name) override;

		virtual CvarParameter* CreateFloatCvar(const char* name, const char* description, double defaultValue, double currentValue, CvarFlags flags = CvarFlags::None) override;
		virtual CvarParameter* CreateIntCvar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue, CvarFlags flags = CvarFlags::None) override;
		virtual CvarParameter* CreateStringCvar(const char* name, const char* description, const char* defaultValue, const char* currentValue, CvarFlags flags = CvarFlags::None) override;
		virtual CvarParameter* CreateBooleanCvar(const char* name, const char* description, bool defaultValue, bool currentValue, CvarFlags flags = CvarFlags::None) override;

		virtual size_t GetFloatCount() const override;
		virtual size_t GetIntCount() const override;
		virtual size_t GetStringCount() const override;

		virtual double* GetFloatCvar(Grindstone::HashedString name) override;
		virtual int32_t* GetIntCvar(Grindstone::HashedString name) override;
		virtual const char* GetStringCvarCstring(Grindstone::HashedString name) override;
		virtual std::string* GetStringCvar(Grindstone::HashedString name) override;

		virtual void SetFloatCvar(Grindstone::HashedString name, double value) override;
		virtual void SetIntCvar(Grindstone::HashedString name, int32_t value) override;
		virtual void SetStringCvar(Grindstone::HashedString name, const char* value) override;

		virtual double* GetFloatCvar(size_t arrayIndex) override;
		virtual int32_t* GetIntCvar(size_t arrayIndex) override;
		virtual const char* GetStringCvarCstring(size_t arrayIndex) override;
		virtual std::string* GetStringCvar(size_t arrayIndex) override;

		virtual void SetFloatCvar(size_t arrayIndex, double value) override;
		virtual void SetIntCvar(size_t arrayIndex, int32_t value) override;
		virtual void SetStringCvar(size_t arrayIndex, const char* value) override;

		virtual Iterator begin() noexcept override;
		virtual Iterator end() noexcept override;
		virtual ConstIterator begin() const noexcept override;
		virtual ConstIterator end() const noexcept override;

		constexpr static size_t MAX_Integer_CVARS = 1000u;
		CvarArray<int32_t> intCvars{ MAX_Integer_CVARS };

		constexpr static int MAX_Float_CVARS = 1000;
		CvarArray<double> floatCvars{ MAX_Float_CVARS };

		constexpr static int MAX_String_CVARS = 200;
		CvarArray<std::string> stringCvars{ MAX_String_CVARS };

		//using templates with specializations to get the cvar arrays for each type.
		//if you try to use a type that doesnt have specialization, it will trigger a linker error
		template<typename T>
		CvarArray<T>* GetCvarArray();

		template<>
		CvarArray<int32_t>* GetCvarArray() {
			return &intCvars;
		}

		template<>
		CvarArray<double>* GetCvarArray() {
			return &floatCvars;
		}

		template<>
		CvarArray<std::string>* GetCvarArray() {
			return &stringCvars;
		}

		//templated get-set cvar versions for syntax sugar
		template<typename T>
		T* GetCvarCurrent(Grindstone::HashedString hash) {
			CvarParameter* par = GetCvar(hash);
			if (!par) {
				return nullptr;
			}
			else {
				return GetCvarArray<T>()->GetCurrentPtr(par->arrayIndex);
			}
		}

		template<typename T>
		void SetCvarCurrent(Grindstone::HashedString hash, const T& value) {
			CvarParameter* cvar = GetCvar(hash);
			if (cvar) {
				GetCvarArray<T>()->SetCurrent(value, cvar->arrayIndex);
			}
		}

		static CvarSystemImpl* GetInstance() {
			return static_cast<CvarSystemImpl*>(CvarSystem::GetInstance());
		}

	private:

		std::shared_mutex mutex;
		CvarParameter* InitCvar(const char* name, const char* description);
		std::unordered_map<Grindstone::HashValue, CvarParameter> savedCvars;
	};
}

CvarSystem* Grindstone::CreateCvarSystemInstance() {
	CvarSystem* cvarSystem = Grindstone::Memory::AllocatorCore::Allocate<CvarSystemImpl>();
	CvarSystem::SetInstance(cvarSystem);
	return cvarSystem;
}

double* CvarSystemImpl::GetFloatCvar(Grindstone::HashedString name) {
	return GetCvarCurrent<double>(name);
}

int32_t* CvarSystemImpl::GetIntCvar(Grindstone::HashedString name) {
	return GetCvarCurrent<int32_t>(name);
}

const char* CvarSystemImpl::GetStringCvarCstring(Grindstone::HashedString name) {
	return GetCvarCurrent<std::string>(name)->c_str();
}

std::string* CvarSystemImpl::GetStringCvar(Grindstone::HashedString name) {
	return GetCvarCurrent<std::string>(name);
}

CvarSystem* CvarSystem::GetInstance() {
	return cvarSystem;
}

void Grindstone::CvarSystem::SetInstance(CvarSystem* ptr) {
	cvarSystem = ptr;
}

CvarParameter* CvarSystemImpl::GetCvar(Grindstone::HashedString name) {
	std::shared_lock lock(mutex);
	auto it = savedCvars.find(name);

	if (it != savedCvars.end()) {
		return &(*it).second;
	}

	return nullptr;
}

void CvarSystemImpl::SetFloatCvar(Grindstone::HashedString name, double value) {
	SetCvarCurrent<double>(name, value);
}

void CvarSystemImpl::SetIntCvar(Grindstone::HashedString name, int32_t value) {
	SetCvarCurrent<int32_t>(name, value);
}

void CvarSystemImpl::SetStringCvar(Grindstone::HashedString name, const char* value) {
	SetCvarCurrent<std::string>(name, value);
}

double* CvarSystemImpl::GetFloatCvar(size_t arrayIndex) {
	return GetCvarArray<double>()->GetCurrentPtr(arrayIndex);
}

int32_t* CvarSystemImpl::GetIntCvar(size_t arrayIndex) {
	return GetCvarArray<int32_t>()->GetCurrentPtr(arrayIndex);
}

const char* CvarSystemImpl::GetStringCvarCstring(size_t arrayIndex) {
	return GetCvarArray<std::string>()->GetCurrentPtr(arrayIndex)->c_str();
}

std::string* CvarSystemImpl::GetStringCvar(size_t arrayIndex) {
	return GetCvarArray<std::string>()->GetCurrentPtr(arrayIndex);
}

void CvarSystemImpl::SetFloatCvar(size_t arrayIndex, double value) {
	*GetCvarArray<double>()->GetCurrentPtr(arrayIndex) = value;
}

void CvarSystemImpl::SetIntCvar(size_t arrayIndex, int32_t value) {
	*GetCvarArray<int32_t>()->GetCurrentPtr(arrayIndex) = value;
}

void CvarSystemImpl::SetStringCvar(size_t arrayIndex, const char* value) {
	*GetCvarArray<std::string>()->GetCurrentPtr(arrayIndex) = value;
}


CvarParameter* CvarSystemImpl::CreateFloatCvar(const char* name, const char* description, double defaultValue, double currentValue, CvarFlags flags) {
	std::unique_lock lock(mutex);
	CvarParameter* param = InitCvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CvarType::Float;

	GetCvarArray<double>()->Add(defaultValue, currentValue, param);

	return param;
}

CvarParameter* CvarSystemImpl::CreateIntCvar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue, CvarFlags flags) {
	std::unique_lock lock(mutex);
	CvarParameter* param = InitCvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CvarType::Integer;

	GetCvarArray<int32_t>()->Add(defaultValue, currentValue, param);

	return param;
}

CvarParameter* CvarSystemImpl::CreateStringCvar(const char* name, const char* description, const char* defaultValue, const char* currentValue, CvarFlags flags) {
	std::unique_lock lock(mutex);
	CvarParameter* param = InitCvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CvarType::String;

	GetCvarArray<std::string>()->Add(defaultValue, currentValue, param);

	return param;
}

CvarParameter* CvarSystemImpl::CreateBooleanCvar(const char* name, const char* description, bool defaultValue, bool currentValue, CvarFlags flags) {
	std::unique_lock lock(mutex);
	CvarParameter* param = InitCvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CvarType::Integer;

	GetCvarArray<int32_t>()->Add(defaultValue, currentValue, param);

	return param;
}

size_t CvarSystemImpl::GetFloatCount() const {
	return CvarSystemImpl::GetInstance()->GetCvarArray<double>()->count;
}

size_t CvarSystemImpl::GetIntCount() const {
	return CvarSystemImpl::GetInstance()->GetCvarArray<int32_t>()->count;
}

size_t CvarSystemImpl::GetStringCount() const {
	return CvarSystemImpl::GetInstance()->GetCvarArray<std::string>()->count;
}

CvarParameter* CvarSystemImpl::InitCvar(const char* name, const char* description) {
	Grindstone::HashValue hashedValue = Grindstone::HashedString{ name };
	savedCvars[hashedValue] = CvarParameter{};

	CvarParameter& newParam = savedCvars[hashedValue];

	newParam.name = name;
	newParam.description = description;

	return &newParam;
}

CvarSystemImpl::Iterator CvarSystemImpl::begin() noexcept {
	return savedCvars.begin();
}

CvarSystemImpl::Iterator CvarSystemImpl::end() noexcept {
	return savedCvars.end();
}

CvarSystemImpl::ConstIterator CvarSystemImpl::begin() const noexcept {
	return savedCvars.begin();
}

CvarSystemImpl::ConstIterator CvarSystemImpl::end() const noexcept {
	return savedCvars.end();
}

template<typename T>
T GetCvarCurrentByIndex(int32_t index) {
	return CvarSystemImpl::GetInstance()->GetCvarArray<T>()->GetCurrent(index);
}

template<typename T>
T& GetRefCvarCurrentByIndex(int32_t index) {
	return *CvarSystemImpl::GetInstance()->GetCvarArray<T>()->GetCurrentPtr(index);
}

template<typename T>
void SetCvarCurrentByIndex(int32_t index, const T& data) {
	CvarSystemImpl::GetInstance()->GetCvarArray<T>()->SetCurrent(data, index);
}
