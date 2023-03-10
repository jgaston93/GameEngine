#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include "System.hpp"
#include "InputMap.hpp"
#include "linmath.h"
#include "Signatures.hpp"

class RenderSystem : public System
{
    public:
    RenderSystem(MessageBus& message_bus, InputMap& input_map);
    ~RenderSystem();

    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);
    void Update(float delta_time);

    private:
    InputMap m_input_map;

    bool m_zoom_on;
    bool m_xray_on;

    int32_t m_shader_program;
    int32_t m_xray_program;
    int32_t m_m_location;
    int32_t m_v_location;
    int32_t m_p_location;
    int32_t m_xray_m_location;
    int32_t m_xray_v_location;
    int32_t m_xray_p_location;
    uint32_t m_texure_sampler_location;

    uint32_t m_texture_slots[7];
    uint32_t m_textures[7];
    int32_t m_texture_sizes[7][2];
    
    vec3 m_eye = { 0, 0, 0 };
    vec3 m_look = { 0, 0, -1 };
    vec3 m_up = { 0, 1, 0 };
};

#endif // RENDER_SYSTEM_HPP