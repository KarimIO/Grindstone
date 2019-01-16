#include "ModelConverter.hpp"
#include "ImageConverter.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "Utilities.hpp"
#include <cstring>

enum OutType {
	OUTPUT_MODEL = 0,
	OUTPUT_TEXTURE,
	OUTPUT_CUBEMAP
};

int main(int argc, char* argv[]) {
	std::vector<OutType> types;
	std::vector<std::string> paths;
	// Check the number of parameters
	if (argc < 2) {
		std::cout << "Enter your command list. Use -c before any cubemap, -t before a texture, and -m before a model: ";
		std::string handle;
		std::getline(std::cin, handle);
		
		std::vector<std::string> tokens;
		unsigned int pos = (unsigned int)handle.find(' ');
		unsigned int prev = 0;

		while (pos != std::string::npos && pos < handle.size()) {
			std::string inst = handle.substr(prev, pos - prev);
			tokens.push_back(inst);
			prev = pos + 1;

			pos = (unsigned int)handle.find(' ', prev);
		}

		std::string inst = handle.substr(prev, ((pos < handle.size()) ? pos : handle.size()) - prev);
		tokens.push_back(inst);

		for (int i = 0; i < tokens.size() - 1; i += 2) {
			if (strcmp("-c", tokens[i].c_str()) == 0) {
				types.push_back(OUTPUT_CUBEMAP);
				paths.push_back(tokens[i + 1]);
			}
			else if (strcmp("-t", tokens[i].c_str()) == 0) {
				types.push_back(OUTPUT_TEXTURE);
				paths.push_back(tokens[i + 1]);
			}
			else if (strcmp("-m", tokens[i].c_str()) == 0) {
				types.push_back(OUTPUT_MODEL);
				paths.push_back(tokens[i + 1]);
			}
		}
	}
	else {
		for (int i = 1; i < argc - 1; i+=2) {
			if (strcmp("-c", argv[i]) == 0) {
				types.push_back(OUTPUT_CUBEMAP);
				paths.push_back(argv[i+1]);
			}
			else if (strcmp("-t", argv[i]) == 0) {
				types.push_back(OUTPUT_TEXTURE);
				paths.push_back(argv[i+1]);
			}
			else if (strcmp("-m", argv[i]) == 0) {
				types.push_back(OUTPUT_MODEL);
				paths.push_back(argv[i+1]);
			}
		}
	}

	std::cout << "Welcome to Grindstone Converter! Handling " << paths.size() <<  " items.\n";

	for (int i = 0; i < paths.size(); i++) {
		OutType type = types.at(i);
		std::string path = paths.at(i);

		std::cout << "Handling " << path << ".\n";

		switch (type) {
		default:
		case OUTPUT_MODEL:
			parseModelConverterParams(path);
			break;
			case OUTPUT_TEXTURE:
				ConvertTexture(path, false, SwapExtension(path, "dds"));
				break;
			case OUTPUT_CUBEMAP:
				ConvertTexture(path, true, SwapExtension(path, "dds"));
				break;
		}
	}

#ifdef _WIN32
	std::cout << "Done. Press enter to quit!" << std::endl;
	std::cin.get();
#endif
	return 0;
}