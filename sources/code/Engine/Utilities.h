#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>

bool ReadFile(std::string path, std::string &output);

bool FileExists(std::string fileName);

char charToLower(char letter);
std::string strToLower(std::string phrase);
char charToUpper(char letter);
std::string strToUpper(std::string phrase);

#endif