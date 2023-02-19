#ifndef LABEL_TEXTURE_HPP
#define LABEL_TEXTURE_HPP

#include <stdint.h>

struct LabelTexture
{
    uint32_t texture_id;
    vec2 texture_size;
    char* characters;
};

#endif // LABEL_TEXTURE_HPP