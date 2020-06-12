#pragma once

#include <vector>
#include <string>
#include "Style.hpp"

namespace Grindstone {
    namespace PluginXmlUi {
        class Stylesheet {
        public:
            bool loadFromFile(std::string path);
            void checkElement();
        private:
            void parse(std::string_view buffer);
            std::vector<Style> styles;
        };
    };
};