#pragma once

#include <filesystem>

#include <PipelineSet/Log.hpp>
#include <PipelineSet/WriteCallback.hpp>

#include "CompilationArtifacts.hpp"
#include "ResolvedStateTree.hpp"

bool OutputPipelineSet(LogCallback logCallback, const CompilationArtifactsGraphics& compilationArtifacts, const ResolvedStateTree::PipelineSet& pipelineSet, PipelineOutput& outputFile);
bool OutputComputeSet(LogCallback logCallback, const CompilationArtifactsCompute& compilationArtifacts, const ResolvedStateTree::ComputeSet& computeSet, PipelineOutput& outputFile);
