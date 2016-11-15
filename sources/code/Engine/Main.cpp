#include <iostream>
#include <stdio.h>
#include <cstring>
#include <Window.h>
#include <Input.h>
#include "Engine.h"

int main(int argc, char *argv[]) {
	std::cout << "Grind Engine Start\n";

	if (!engine.Initialize()) {
#ifdef _WIN32
		system("pause");
#endif
		return -1;
	}
		
	engine.Run();
	engine.Shutdown();

	return 0;
}