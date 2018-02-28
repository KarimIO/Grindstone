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