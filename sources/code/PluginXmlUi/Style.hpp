#pragma once

namespace Grindstone {
    namespace PluginXmlUi {
        struct Color {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        struct Style {
            Color color;
            Color background_coloor;
        };
    };
};