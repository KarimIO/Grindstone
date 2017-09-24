#include "Utilities.h"
#include <fstream>
#include <string>

bool ReadFileIncludable(std::string pFileName, std::string& output)
{
	std::ifstream file;
	file.open(pFileName);

	if (!file.fail()) {
		std::string line;
		while (getline(file, line)) {
			if (line.substr(0, 8) == "#include") {
				// Load the Include
				std::string outTemp;
				ReadFileIncludable((line.substr(9)).c_str(), outTemp);
				output += outTemp;
			}
			else {
				output += line + "\n";
			}
		}

		file.close();
		return true;
	}
	else {
		printf("Unable to open file %s\n", pFileName.c_str());
	}

	return false;
}

bool ReadFile(std::string pFileName, std::string& output)
{
	std::ifstream file;
	file.open(pFileName);

	if (!file.fail()) {
		std::string line;
		while (getline(file, line))
			output += line + "\n";

		file.close();
		return true;
	}
	else {
		printf("Unable to open file %s\n", pFileName.c_str());
	}

	return false;
}

bool FileExists(std::string fileName) {
	std::ifstream infile(fileName.c_str());
	return infile.good();
}

char charToLower(char letter) {
	int charNum = (int)letter;
	if (charNum >= 65 && charNum <= 90)
		charNum += 32;

	return (char)charNum;
}

std::string strToLower(std::string phrase) {
	std::string out = "";
	for (size_t i = 0; i < phrase.size(); i++) {
		out += charToLower(phrase[i]);
	}
	return out;
}

char charToUpper(char letter) {
	int charNum = (int)letter;
	if (charNum >= 97 && charNum <= 122)
		charNum -= 32;

	return (char)charNum;
}

std::string strToUpper(std::string phrase) {
	std::string out = "";
	for (size_t i = 0; i < phrase.size(); i++) {
		out += charToUpper(phrase[i]);
	}
	return out;
}