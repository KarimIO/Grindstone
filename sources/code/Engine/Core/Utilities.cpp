#include "Utilities.hpp"
#include <fstream>
#include <string>
#include <iostream>

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
		GRIND_ERROR("Unable to open file {0}", pFileName.c_str());
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
		GRIND_ERROR("Unable to open file {0}", pFileName.c_str());
	}

	return false;
}

bool readFileBinary(const std::string& filename, std::vector<char>& buffer) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	size_t file_size = (size_t)file.tellg();
	buffer.resize(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();

	return true;
}

bool readFile(const std::string& filename, std::vector<char>& buffer) {
	std::ifstream file;


	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		file.open(filename, std::ios::binary | std::ios::ate);
	}
	catch (std::system_error& e) {
		std::cerr << e.code().message() << std::endl;
	}

	if (!file.is_open()) {
		return false;
	}

	size_t file_size = (size_t)file.tellg();
	buffer.resize(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();

	return true;
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

std::istream& safeGetline(std::istream& is, std::string& t) {
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case std::streambuf::traits_type::eof():
			// Also handle the case when the last line has no line ending
			if (t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
		}
	}
}