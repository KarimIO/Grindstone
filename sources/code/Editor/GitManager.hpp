#pragma once

#include <filesystem>
#include <string>

struct git_repository;

namespace Grindstone {
	namespace Editor {
		enum class GitRepoStatus {
			NeedCheck = 0,
			NoRepo,
			RepoInitializedButUnchecked,
			RepoMatched,
			RepoUnmatched
		};

		class GitManager {
		public:
			void Initialize();
			void UpdateGitPeriodically();
			GitRepoStatus GetGitRepoStatus();
			std::string& GetBranchName();
			void Fetch();
			void Pull();
			void Push();
			bool OpenRepository();
			void CloseRepository();
		private:
			void UpdateGit();
			void UpdateBranchName();
			GitRepoStatus repoStatus = GitRepoStatus::NeedCheck;
			git_repository* repo = nullptr;
			std::string currentBranchName;
		};
	}
}
