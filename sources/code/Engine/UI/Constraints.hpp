#pragma once

enum class ConstraintValueType {
    Pixel = 0,
    Percentage
};

enum class ConstraintPositioning {
    Stretch = 0,
    Min,
    Mid,
    Max
};

struct Constraint {
    ConstraintValueType value_type;
    union value {
        int pixel;
        float percentage;
    };
};