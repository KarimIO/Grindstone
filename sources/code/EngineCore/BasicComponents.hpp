#pragma once

namespace Grindstone {
    struct TransformComponent {
        float position_[3];
        float angles_[3];
        float scale_[3];

        float world_[4][4];
    };
}