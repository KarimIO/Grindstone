#pragma once

#include "Stylesheet.hpp"
#include <fstream>
#include <iostream>

using namespace Grindstone::PluginXmlUi;

bool Stylesheet::loadFromFile(std::string path) {
    std::ifstream file(path, std::ios::ate);

    if (file.fail()) {
        return false;
    }
    else {
        std::string buffer;
		size_t file_size = (size_t)file.tellg();
		buffer.resize(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);
        parse(buffer);

		file.close();
        return true;
    }
}

void Stylesheet::checkElement() {

}

void Stylesheet::parse(std::string_view buffer) {
    std::cout << buffer << std::endl;
}