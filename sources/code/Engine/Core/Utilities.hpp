#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <string>
#include <vector>

bool ReadFile(std::string path, std::string &output);
bool ReadFileIncludable(std::string path, std::string &output);

bool FileExists(std::string fileName);

char charToLower(char letter);
std::string strToLower(std::string phrase);
char charToUpper(char letter);
std::string strToUpper(std::string phrase);
std::istream& safeGetline(std::istream& is, std::string& t);

bool readFileBinary(const std::string& filename, std::vector<char>& buffer);
bool readFile(const std::string& filename, std::vector<char>& buffer);

#endif