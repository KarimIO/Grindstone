#pragma once

#include "UiNode.hpp"
#include <string>

namespace Grindstone {
    namespace Ui {
        struct Text : public Node {
            std::string text_;
        }; // Node
    } // namespace Ui
} // namespace Grindstone
