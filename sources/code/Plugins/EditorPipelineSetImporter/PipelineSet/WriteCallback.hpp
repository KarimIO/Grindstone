#pragma once

#include <filesystem>
#include <string_view>

using WriteCallback = std::function<void(std::filesystem::path, size_t, void*)>;
