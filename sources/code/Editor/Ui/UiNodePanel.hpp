#pragma once

#include "UiNode.hpp"
#include <string>

namespace Grindstone {
    namespace Ui {
        struct Panel : public Node {
            std::string tag_;
        }; // Node
    } // namespace Ui
} // namespace Grindstone
