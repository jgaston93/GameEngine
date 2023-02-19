#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "System.hpp"
#include "InputMap.hpp"
#include "linmath.h"
#include "Signatures.hpp"

struct VertexData
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

class RenderSystem : public System
{
    public:
    RenderSystem(MessageBus& message_bus, uint32_t mvp_location);
    ~RenderSystem();

    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);
    void Update(float delta_time);

    private:

    uint32_t m_mvp_location;
    
    vec3 m_eye = { 0, 0, 0 };
    vec3 m_look = { 0, 0, -1 };
    vec3 m_up = { 0, 1, 0 };
};

#endif // RENDER_SYSTEM_HPP