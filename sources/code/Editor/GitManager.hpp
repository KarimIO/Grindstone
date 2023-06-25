#pragma once

#include <filesystem>
#include <string>

struct git_repository;

namespace Grindstone {
	namespace Editor {
		enum class GitRepoStatus {
			NeedCheck = 0,
			NoRepo,
			RepoInitializedButUnfetched,
			RepoFetched
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
			uint32_t GetBehindCount();
			uint32_t GetAheadCount();
			uint32_t GetChangesCount();
		private:
			void UpdateGit();
			void UpdateBranchName();
			GitRepoStatus repoStatus = GitRepoStatus::NeedCheck;
			git_repository* repo = nullptr;
			std::string currentBranchName;
			uint32_t behindCount = 0;
			uint32_t aheadCount = 0;
			uint32_t changesCount = 0;
		};
	}
}
