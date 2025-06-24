#pragma once

#include <PipelineSet/Log.hpp>
#include "Token.hpp"

bool ScanPipelineSet(LogCallback logCallback, const std::filesystem::path& path, std::string_view content, TokenList& outScannerTokens);
