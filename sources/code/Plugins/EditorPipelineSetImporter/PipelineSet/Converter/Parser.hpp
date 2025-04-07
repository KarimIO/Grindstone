#pragma once

#include <filesystem>

#include <PipelineSet/Log.hpp>
#include "Token.hpp"
#include "ParseTree.hpp"

bool ParsePipelineSet(LogCallback logCallback, const std::filesystem::path& path, TokenList& scannerTokens, ParseTree& outParseData);
