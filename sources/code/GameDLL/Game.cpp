#include <iostream>

extern "C" {
	void __declspec(dllexport) launchGame() {
		std::cout << "Launch Game\r\n";

		while (true) {
			std::cout << "Running\r\n";
		}

		std::cout << "End Game\r\n";
	}
}