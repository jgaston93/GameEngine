#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <stdint.h>

#include "linmath.h"

struct Animation
{
    bool paused;
    float speed;
    float counter;
    uint32_t num_frames;
    uint32_t current_frame;
};

#endif // ANIMATION_HPP