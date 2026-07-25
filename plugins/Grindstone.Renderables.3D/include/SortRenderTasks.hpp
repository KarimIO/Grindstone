#pragma once

#include <vector>
#include <algorithm>

namespace Grindstone::Renderer {
	template<typename RenderTask>
	int CompareRenderSort(const RenderTask& a, const RenderTask& b) {
		return a.sortData > b.sortData;
	}

	template<typename RenderTask>
	int CompareReverseRenderSort(const RenderTask& a, const RenderTask& b) {
		return a.sortData < b.sortData;
	}

	template<typename RenderTask>
	void SortRenderTasks(std::vector<RenderTask>& renderTasks) {
		std::sort(renderTasks.begin(), renderTasks.end(), CompareRenderSort<RenderTask>);
	}

	template<typename RenderTask>
	void SortRenderTasksReverse(std::vector<RenderTask>& renderTasks) {
		std::sort(renderTasks.begin(), renderTasks.end(), CompareReverseRenderSort<RenderTask>);
	}
}
