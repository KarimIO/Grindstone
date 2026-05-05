#pragma once

#define GS_ENUM_FLAGS_FUNCS(T) \
	inline T operator~(const T stages) {\
		using UnderlyingType = std::underlying_type_t<T>;\
		return static_cast<T>(~static_cast<UnderlyingType>(stages));\
	}\
	\
	inline T operator|(const T a, const T b) {\
		using UnderlyingType = std::underlying_type_t<T>;\
		return static_cast<T>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b));\
	}\
	\
	inline T operator&(const T a, const T b) {\
		using UnderlyingType = std::underlying_type_t<T>;\
		return static_cast<T>(static_cast<UnderlyingType>(a) & static_cast<UnderlyingType>(b));\
	}\
	\
	inline T operator^(const T a, const T b) {\
		using UnderlyingType = std::underlying_type_t<T>;\
		return static_cast<T>(static_cast<UnderlyingType>(a) ^ static_cast<UnderlyingType>(b));\
	}\
	\
	inline T& operator|=(T& a, const T b) {\
		a = a | b;\
		return a;\
	}\
	\
	inline T& operator&=(T& a, const T b) {\
		a = a & b;\
		return a;\
	}\
	\
	inline T& operator^=(T& a, const T b) {\
		a = a ^ b;\
		return a;\
	}\
	\
	inline bool Any(T v) {\
		using UnderlyingType = std::underlying_type_t<T>;\
		return static_cast<UnderlyingType>(v) != 0;\
	}

template <typename Enum>
struct EnumTraits {
	static constexpr const char* names[] = {};
	static constexpr size_t size = 0;
};

template <typename Enum>
struct EnumFlagsTraits {
	static constexpr const char* names[] = {};
	static constexpr size_t size = 0;
};
