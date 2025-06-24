#pragma once

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
