#include "Utilities.h"
#include <fstream>
#include <string>

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
		printf("Unable to open file %s", pFileName.c_str());
	}
	
	return false;
}

bool FileExists(std::string fileName) {
	std::ifstream infile(fileName.c_str());
	return infile.good();
}