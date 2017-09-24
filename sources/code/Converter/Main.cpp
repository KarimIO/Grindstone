#include "ModelConverter.h"
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
}

int main(int argc, char* argv[]) {
	std::string path;
	// Check the number of parameters
#if 0
	if (argc < 2) {
		std::cout << "Enter path to file: ";
		std::cin >> path;
	}
	else {
		path = argv[0];
	}
#else
	path = "../assets/models/materialTest/materialTest.obj";
#endif
	
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