#include "ModelConverter.hpp"
#include "ImageConverter.hpp"
#include "AnimationConverter.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "Utilities.hpp"
#include <cstring>
#include <queue>

void handleArgInput(std::queue<std::string> &args) {
	std::cout << "> ";
	std::string handle;
	std::getline(std::cin, handle);

	// Push as long as it's not a quit
	if (handle != "q" && handle != "quit") {
		args.push(handle);
	}
}

void getHelp() {
	std::cout << "We support the following commands: \n";
	std::cout << "anim <path>\n";
	std::cout << "cube <path>\n";
	std::cout << "img <path>\n";
	std::cout << "mdl <path>\n";
}

int main(int argc, char* argv[]) {
	std::queue<std::string> args;

	// Push all tokens to supertoken.
	if (argc > 1) {
		std::string arg;
		for (int i = 1; i < argc; ++i) {
			arg = argv[i];
			if (i != argc - 1)
				arg += " ";
		}
		args.push(arg);
	}

	std::cout << "Welcome to Grindstone Converter!\nType help for more info.\n";

	// If we don't have any input, then simply poll for it
	if (args.size() == 0)
		handleArgInput(args);

	while (args.size() > 0) {
		std::string arg = args.front();
		args.pop();

		std::cout << "Handling " << arg << ".\n";

		std::string arg3 = arg.substr(0, 3);
		std::string arg4 = arg.substr(0, 4);

		try {
			if (arg3 == "mdl") {
				parseModelConverterParams(arg);
			}
			else if (arg3 == "img") {
				ConvertTexture(arg.substr(4), false, SwapExtension(arg.substr(4), "dds"));
			}
			else if (arg4 == "cube") {
				ConvertTexture(arg.substr(5), true, SwapExtension(arg.substr(5), "dds"));
			}
			else if (arg4 == "anim") {
				parseAnimationConverterParams(arg);
			}
			else if (arg4 == "help") {
				getHelp();
			}
			else {
				std::cout << "Invalid file type.\n";
			}
		}
		catch (std::runtime_error err) {
			std::cout << err.what() << "\n";
		}

		std::cout << "\n";

		handleArgInput(args);
	}

	return 0;
}