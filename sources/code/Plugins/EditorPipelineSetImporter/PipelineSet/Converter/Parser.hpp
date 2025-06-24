#pragma once

#include <set>
#include <filesystem>

#include <PipelineSet/Log.hpp>
#include "Token.hpp"
#include "ParseTree.hpp"

bool ParsePipelineSet(LogCallback logCallback, const std::filesystem::path& path, TokenList& scannerTokens, ParseTree& outParseData, std::set<std::filesystem::path>& unprocessedFiles);
