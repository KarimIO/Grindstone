#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>

bool CopyFileTo(std::string path, std::string to);
bool CreateFolder(const char * path);
std::string SwapExtension(std::string path, std::string ext);
std::string sanitizeFileName(std::string name);
void switchSlashes(std::string &path);
std::string extractFilename(std::string path);
std::string extractFilenameAndExtension(std::string path);

#endif