#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>

bool CopyFileTo(std::string path, std::string to);
bool CreateFolder(const char * path);
std::string SwapExtension(std::string path, std::string ext);

#endif