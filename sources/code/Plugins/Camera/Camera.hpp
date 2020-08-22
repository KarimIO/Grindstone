#pragma once

namespace Grindstone {
    struct CameraComponent {
        bool is_orthographic_;
        float near_;
        float far_;
        float fov_;
        float aspect_ratio_;
    };
}