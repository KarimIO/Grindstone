#include <git2/repository.h>
#include <git2/refs.h>
#include <git2/graph.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/remote.h>
#include <git2/branch.h>

#include "GitManager.hpp"
#include "EditorManager.hpp"

using namespace Grindstone::Editor;

void GitManager::Initialize() {
	currentBranchName = "";
	repoStatus = GitRepoStatus::NeedCheck;
	UpdateGit();
}

std::string& GitManager::GetBranchName() {
	return currentBranchName;
}

GitRepoStatus GitManager::GetGitRepoStatus() {
	return repoStatus;
}

void GitManager::UpdateBranchName() {
	currentBranchName = "";
	git_reference* reference = nullptr;
	if (git_repository_head(&reference, repo) != 0) {
		return;
	}

	const char* branchName = nullptr;
	if (git_branch_name(&branchName, reference) != 0) {
		return;
	}

	currentBranchName = branchName;

	git_reference_free(reference);
}

void GitManager::UpdateGitPeriodically() {
	UpdateGit();
}

void GitManager::UpdateGit() {
	git_libgit2_init();

	if (repo == nullptr) {
		if (!OpenRepository()) {
			return;
		}
	}

	UpdateBranchName();
	Fetch();
}

void GitManager::Fetch() {
	git_oid local, upstream;
	std::string localName = "refs/heads/" + currentBranchName;
	std::string upstreamName = "refs/remotes/origin/" + currentBranchName;
	git_reference_name_to_id(&local, repo, localName.c_str());
	git_reference_name_to_id(&upstream, repo, upstreamName.c_str());

	size_t ahead, behind;
	if (git_graph_ahead_behind(&ahead, &behind, repo, &local, &upstream) != 0) {
		return;
	}

	aheadCount = static_cast<uint32_t>(ahead);
	behindCount = static_cast<uint32_t>(behind);
	repoStatus = GitRepoStatus::RepoFetched;
}

void GitManager::Pull() {

}

void GitManager::Push() {

}

bool GitManager::OpenRepository() {
	if (repo != nullptr) {
		CloseRepository();
	}

	auto& editorManager = Editor::Manager::GetInstance();
	std::filesystem::path projectPath = editorManager.GetProjectPath();
	std::string projectPathString = projectPath.string();
	const char* path = projectPathString.c_str();

	git_repository* newRepo = nullptr;
	bool isSuccessful = git_repository_open(&newRepo, path) == 0;

	if (isSuccessful) {
		repo = newRepo;
		repoStatus = GitRepoStatus::RepoInitializedButUnfetched;
	}
	else {
		repoStatus = GitRepoStatus::NoRepo;
	}

	return isSuccessful;
}

void GitManager::CloseRepository() {
	git_repository_free(repo);
	repoStatus = GitRepoStatus::NeedCheck;
	repo = nullptr;
}

uint32_t GitManager::GetBehindCount() {
	return behindCount;
}

uint32_t GitManager::GetAheadCount() {
	return aheadCount;
}

uint32_t GitManager::GetChangesCount() {
	return changesCount;
}
