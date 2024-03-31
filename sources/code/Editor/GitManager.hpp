#pragma once

#include <filesystem>
#include <string>

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
			void Fetch();
			void Pull();
			void Push();
			bool OpenRepository();
			void CloseRepository();

			GitRepoStatus GetGitRepoStatus() const;
			const std::string& GetBranchName() const;
			uint32_t GetBehindCount() const;
			uint32_t GetAheadCount() const;
			uint32_t GetChangesCount() const;
		private:
			void UpdateGit();
			void UpdateBranchName();

			struct git_repository* repo = nullptr;

			GitRepoStatus repoStatus = GitRepoStatus::NeedCheck;
			std::string currentBranchName;
			uint32_t behindCount = 0;
			uint32_t aheadCount = 0;
			uint32_t changesCount = 0;
		};
	}
}
