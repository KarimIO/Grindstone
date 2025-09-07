#pragma once

#include <Grindstone.Editor.PipelineSetImporter/include/PipelineSet/Log.hpp>
#include <Grindstone.Editor.PipelineSetImporter/include/PipelineSet/Converter/CompilationOptions.hpp>

#include "ResolvedStateTree.hpp"
#include "CompilationArtifacts.hpp"

bool CompileShadersGraphics(LogCallback logCallback, const ResolvedStateTree::PipelineSet& parseDataSet, CompilationOptions& options, CompilationArtifactsGraphics& output);
bool CompileShadersCompute(LogCallback logCallback, const ResolvedStateTree::ComputeSet& parseDataSet, CompilationOptions& options, CompilationArtifactsCompute& output);
