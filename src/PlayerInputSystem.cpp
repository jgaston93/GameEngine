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
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);

    float player_velocity = 4;
    float player_rotation_velocity = 2000;

    rigid_body.velocity[0] = 0;
    rigid_body.velocity[2] = 0;

    double delta_mouse_x = m_input_map.GetMousePosX() - m_prev_mouse_pos_x;
    double delta_mouse_y = m_input_map.GetMousePosY() - m_prev_mouse_pos_y;
    if(delta_mouse_x > player_rotation_velocity)
    {
        delta_mouse_x = player_rotation_velocity;
    }
    else if(delta_mouse_x < -player_rotation_velocity)
    {
        delta_mouse_x = -player_rotation_velocity;
    }
    if(delta_mouse_y > player_rotation_velocity)
    {
        delta_mouse_y = player_rotation_velocity;
    }
    else if(delta_mouse_y < -player_rotation_velocity)
    {
        delta_mouse_y = -player_rotation_velocity;
    }
    transform.rotation[1] -= delta_mouse_x * delta_time;
    transform.rotation[0] -= delta_mouse_y * delta_time;
    m_prev_mouse_pos_x = m_input_map.GetMousePosX();
    m_prev_mouse_pos_y = m_input_map.GetMousePosY();

    if(transform.rotation[0] < -45)
    {
        transform.rotation[0] = -45;
    }
    else if(transform.rotation[0] > 45)
    {
        transform.rotation[0] = 45;
    }

    vec4 velocity = { 0, 0, 0, 1.0 };

    if(m_input_map.IsPressed(GLFW_KEY_W))
    {
        velocity[2] -= player_velocity;
    }
    else if(m_input_map.IsPressed(GLFW_KEY_S))
    {
        velocity[2] += player_velocity;
    }

    if(m_input_map.IsPressed(GLFW_KEY_A))
    {
        velocity[0] -= player_velocity;
    }
    else if(m_input_map.IsPressed(GLFW_KEY_D))
    {
        velocity[0] += player_velocity;
    }

    mat4x4 rotation_matrix;
    mat4x4_identity(rotation_matrix);
    mat4x4_rotate_Y(rotation_matrix, rotation_matrix, transform.rotation[1] * M_PI / 180.0);
    vec4 rotated_velocity;
    mat4x4_mul_vec4(rotated_velocity, rotation_matrix, velocity);

    rigid_body.velocity[0] = rotated_velocity[0];
    rigid_body.velocity[1] = rotated_velocity[1];
    rigid_body.velocity[2] = rotated_velocity[2];
}