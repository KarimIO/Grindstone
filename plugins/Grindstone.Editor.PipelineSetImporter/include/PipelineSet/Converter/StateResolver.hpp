#pragma once

#include <Grindstone.Editor.PipelineSetImporter/include/PipelineSet/Log.hpp>
#include "ResolvedStateTree.hpp"
#include "ParseTree.hpp"

bool ResolveStateTree(LogCallback logCallback, const ParseTree& parseTree, ResolvedStateTree& resolvedStateTree);
