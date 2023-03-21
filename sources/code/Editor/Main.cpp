#include <iostream>
#include "EditorManager.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>
#include <shobjidl.h>

// Request High-Performance GPU for Nvidia and AMD
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::string FindFolder() {
	std::string outString;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen = nullptr;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions)))
			{
				pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}

			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						std::wstring tempWStr(pszFilePath);
						outString = std::string(tempWStr.begin(), tempWStr.end());
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return outString;
}

std::string CreateNewProject() {
	std::filesystem::path basePath;

	do {
		basePath = FindFolder();
	} while (!std::filesystem::is_empty(basePath));

	std::filesystem::create_directories(basePath / "assets");
	std::filesystem::create_directories(basePath / "log");
	std::filesystem::create_directories(basePath / "buildSettings");
	std::filesystem::create_directories(basePath / "compiledAssets");
	std::filesystem::create_directories(basePath / "userSettings");

	return basePath.string();
}

std::string OpenExistingProject() {
	return FindFolder();

}
#endif

int main(int argc, char* argv[]) {
	std::string projectPath;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-projectpath") == 0 && argc > i + 1) {
			projectPath = argv[i + 1];
		}
	}

	if (projectPath.empty()) {
#if _WIN32
		int msgboxID = MessageBox(
			NULL,
			"Do you want to open an existing project? Press no to create a new project.",
			"Project Setup",
			MB_ICONEXCLAMATION | MB_YESNOCANCEL
		);

		switch (msgboxID) {
		case IDYES:
			projectPath = CreateNewProject();
			break;
		case IDNO:
			projectPath = OpenExistingProject();
			break;
		case IDCANCEL:
			return 1;
		}
#else
		std::cerr << "Unable to launch Grindstone Editor - no path set." << std::endl;
		return 1;
#endif
	}

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	if (editorManager.Initialize(projectPath.c_str())) {
		editorManager.Run();
	}

	return 0;
}
