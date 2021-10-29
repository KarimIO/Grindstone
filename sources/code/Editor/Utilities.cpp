#include "Utilities.hpp"
#include <fstream>

#ifdef _WIN32
#include <windows.h>

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool CreateFolder(const char * path) {
	if (!dirExists(path)) {
		return CreateDirectory(path, NULL);
	}
	return true;
}
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st = { 0 };

bool CreateFolder(const char * path) {
	if (stat(path, &st) == -1) {
		int result = mkdir(path, 0700);
		return result == 0;
	}
	return true;
}
#endif

bool CopyFileTo(std::string path, std::string to) {
	std::ifstream  src(path, std::ios::binary);
	if (src.fail()) {
		return false;
	}

	std::ofstream  dst(to, std::ios::binary);
	if (dst.fail()) {
		return false;
	}

	dst << src.rdbuf();
	return true;
}

std::string SwapExtension(std::string path, std::string ext) {
	size_t p = path.find_last_of(".");
	return path.substr(0, p+1) + ext;
}



std::string sanitizeFileName(std::string name) {
	const std::string illegal = "<>/\\/\":";
	for (auto c = name.begin(); c < name.end(); ++c) {
		if (illegal.find(*c) != std::string::npos) {
			*c = ' ';
		}
	}

	return name;
}

void switchSlashes(std::string &path) {
	size_t index = 0;
	while (true) {
		// Locate the substring to replace.
		index = path.find("\\", index);
		if (index == std::string::npos) break;

		// Make the replacement.
		path.replace(index, 1, "/");

		// Advance index forward so the next iteration doesn't pick it up as well.
		index += 1;
	}
}

std::string extractFilename(std::string path) {
	path = extractFilenameAndExtension(path);
	return path.substr(0, path.find_last_of("."));
	return std::string();
}

std::string extractFilenameAndExtension(std::string path) {
	return path.substr(path.find_last_of("/") + 1);
}
