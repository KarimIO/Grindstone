#include "ModelConverter.hpp"
#include <iostream>
#include <string>

enum OutType {
	OUTPUT_MODEL
};

OutType convertString(std::string input) {
	size_t p = input.find_last_of(".");
	std::string base = input.substr(0, p + 1);
	std::string ext = input.substr(p + 1);
	if (ext == "fbx")
		return OUTPUT_MODEL;
	else if (ext == "obj")
		return OUTPUT_MODEL;

	return OUTPUT_MODEL;
}

int main(int argc, char* argv[]) {
	std::string path;
	// Check the number of parameters
	if (argc < 2) {
		std::cout << "Enter path to file: ";
		std::getline(std::cin, path);
		if (path == "crytek") {
			path = "../assets/models/crytek-sponza/sponza.obj";
		}
	}
	else {
		path = argv[1];
	}

	OutType type = convertString(path);
	//switch (type) {
	//case OUTPUT_MODEL:
		ModelConverter(path);
	//	break;
	//}

	//LoadModelFile("../assets/models/guard.gmf");

#ifdef _WIN32
	system("pause");
#endif
	return 0;
}