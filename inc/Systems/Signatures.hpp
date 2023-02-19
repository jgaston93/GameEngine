#ifndef SIGNATURES_HPP
#define SIGNATURES_HPP

#include <stdint.h>

const uint32_t ANIMATION_SYSTEM_SIGNATURE =    0x00000001;
const uint32_t AI_SYSTEM_SIGNATURE =           0x00000002;
const uint32_t PHYSICS_SYSTEM_SIGNATURE =      0x00000004;
const uint32_t COLLISION_SYSTEM_SIGNATURE =    0x00000008;
const uint32_t PLAYER_INPUT_SYSTEM_SIGNATURE = 0x00000010;
const uint32_t RENDER_SYSTEM_SIGNATURE =       0x00000020;
const uint32_t HUD_RENDER_SYSTEM_SIGNATURE =   0x00000040;
const uint32_t TIMER_SYSTEM_SIGNATURE =        0x00000080;
const uint32_t BOUNDS_SYSTEM_SIGNATURE =       0x00000100;

#endif // SIGNATURES_HPP