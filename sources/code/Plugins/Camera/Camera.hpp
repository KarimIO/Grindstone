#pragma once

namespace Grindstone {
    struct CameraComponent {
        bool isOrthographic;
        float near;
        float far;
        float fov;
        float aspectRatio;
    };
}