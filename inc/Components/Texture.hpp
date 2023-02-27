#ifndef TEXTURE_HPP
#define TEXTURE_HPP

struct Texture
{
    uint32_t texture_index;
    vec2 position;
    vec2 size;
    vec3 color;
    bool use_light;
};

#endif // TEXTURE_HPP