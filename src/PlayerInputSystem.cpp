#include <cstdio>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "PlayerInputSystem.hpp"


PlayerInputSystem::PlayerInputSystem(MessageBus& message_bus, InputMap& input_map) : 
    System(message_bus, PLAYER_INPUT_SYSTEM_SIGNATURE), 
    m_input_map(input_map)
{

}

PlayerInputSystem::~PlayerInputSystem()
{
}

void PlayerInputSystem::HandleMessage(Message message)
{

}

void PlayerInputSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);

    float player_acceleration = 0.5;

    rigid_body.acceleration[0] = 0;
    rigid_body.acceleration[1] = 0;

    if(m_input_map.IsPressed(GLFW_KEY_LEFT))
    {
        rigid_body.acceleration[0] -= player_acceleration;
    }
    else if(m_input_map.IsPressed(GLFW_KEY_RIGHT))
    {
        rigid_body.acceleration[0] += player_acceleration;
    }

    if(m_input_map.IsPressed(GLFW_KEY_UP))
    {
        rigid_body.acceleration[1] += player_acceleration;
    }
    else if(m_input_map.IsPressed(GLFW_KEY_DOWN))
    {
        rigid_body.acceleration[1] -= player_acceleration;
    }
}