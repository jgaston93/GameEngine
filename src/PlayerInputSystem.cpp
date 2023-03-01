#include <cstdio>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "PlayerInputSystem.hpp"


PlayerInputSystem::PlayerInputSystem(MessageBus& message_bus, InputMap& input_map) : 
    System(message_bus, PLAYER_INPUT_SYSTEM_SIGNATURE), 
    m_input_map(input_map),
    m_zoom_on(false),
    m_xray_on(false),
    m_shoot_timer(0),
    m_num_bullets(32),
    m_bullet_index(0)
{

}

PlayerInputSystem::~PlayerInputSystem()
{
}

void PlayerInputSystem::HandleMessage(Message message)
{
    if(message.message_type == MessageType::COLLISION)
    {
        uint32_t entity_1_id = message.message_data >> 16;
        uint32_t entity_2_id = message.message_data & 0x0000FFFF;

        char* entity_1_tag = m_entity_manager->GetEntityTag(entity_1_id);
        char* entity_2_tag = m_entity_manager->GetEntityTag(entity_2_id);

        if(strncmp(entity_1_tag, "bullet", 6) == 0)
        {
            uint32_t bullet_id = m_entity_manager->GetEntityId(entity_1_tag);
            m_entity_manager->SetEntityState(bullet_id, EntityState::INACTIVE);
            if(strcmp(entity_2_tag, "enemy") == 0)
            {
                uint32_t player_id = m_entity_manager->GetEntityId("player");
                PlayerInput& player_input = m_component_manager->GetComponent<PlayerInput>(player_id);
                player_input.score += 1;
            }
        }
    }
}

void PlayerInputSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    PlayerInput& player_input = m_component_manager->GetComponent<PlayerInput>(entity_id);
    uint32_t timer_entity_id = m_entity_manager->GetEntityId("timer_entity");
    uint32_t win_entity_id = m_entity_manager->GetEntityId("win_entity");
    uint32_t lose_entity_id = m_entity_manager->GetEntityId("lose_entity");

    if(player_input.state == PlayerState::INIT)
    {
        if(m_input_map.IsPressed(GLFW_KEY_SPACE))
        {
            player_input.state = PlayerState::RUNNING;
            uint32_t title_entity_id = m_entity_manager->GetEntityId("title_entity");
            uint32_t timer_entity_id = m_entity_manager->GetEntityId("timer_entity");
            m_entity_manager->SetEntityState(title_entity_id, EntityState::INACTIVE);
            m_entity_manager->SetEntityState(timer_entity_id, EntityState::ACTIVE);
        }
    }
    else if(player_input.state == PlayerState::RUNNING)
    {
        if(player_input.score == 4)
        {
            player_input.state = PlayerState::GAMEOVER;
            m_entity_manager->SetEntityState(win_entity_id, EntityState::ACTIVE);
            return;
        }
        player_input.timer -= delta_time;
        Label& label = m_component_manager->GetComponent<Label>(timer_entity_id);
        sprintf(label.text, "%d", (int)player_input.timer);
        if(player_input.timer <= 0)
        {
            player_input.state = PlayerState::GAMEOVER;
            m_entity_manager->SetEntityState(lose_entity_id, EntityState::ACTIVE);
            return;
        }
        
        // Handle zoom and xray commands
        Message message;
        bool send_message = false;
        if(!m_xray_on && m_input_map.IsPressed(GLFW_KEY_E) && !m_zoom_on)
        {
            message.message_type = MessageType::XRAY;
            message.message_data = 1;
            m_xray_on = true;
            send_message = true;
        }
        else if(m_xray_on && !m_input_map.IsPressed(GLFW_KEY_E))
        {
            message.message_type = MessageType::XRAY;
            message.message_data = 0;
            m_xray_on = false;
            send_message = true;
        }

        uint32_t crosshair_entity_id = m_entity_manager->GetEntityId("crosshair");
        
        if(!m_zoom_on && m_input_map.IsPressed(GLFW_MOUSE_BUTTON_RIGHT) && !m_xray_on)
        {
            message.message_type = MessageType::ZOOM;
            message.message_data = 1;
            m_zoom_on = true;
            send_message = true;
            m_entity_manager->SetEntityState(crosshair_entity_id, EntityState::ACTIVE);
        }
        else if(m_zoom_on && !m_input_map.IsPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            message.message_type = MessageType::ZOOM;
            message.message_data = 0;
            m_zoom_on = false;
            send_message = true;
            m_entity_manager->SetEntityState(crosshair_entity_id, EntityState::INACTIVE);
        }

        if(send_message)
        {
            m_message_bus.PostMessage(message);
        }

        
        RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
        Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);

        // Handle bullet stuff
        if(m_shoot_timer > 0)
        {
            m_shoot_timer -= delta_time;
        }
        if(m_zoom_on && m_shoot_timer <= 0 && m_input_map.IsPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            m_shoot_timer = 1;
            char bullet_tag[10];
            sprintf(bullet_tag, "bullet_%d", m_bullet_index);
            uint32_t bullet_id = m_entity_manager->GetEntityId(bullet_tag);
            Transform& bullet_transform = m_component_manager->GetComponent<Transform>(bullet_id);
            RigidBody& bullet_rigid_body = m_component_manager->GetComponent<RigidBody>(bullet_id);

            bullet_transform.position[0] = transform.position[0];
            bullet_transform.position[1] = transform.position[1];
            bullet_transform.position[2] = transform.position[2];

            vec4 bullet_velocity = { 0.0, 0.0, -200.0, 1.0 };

            mat4x4 rotation_matrix;
            mat4x4_identity(rotation_matrix);
            mat4x4_rotate_Z(rotation_matrix, rotation_matrix, transform.rotation[2] * M_PI / 180.0);
            mat4x4_rotate_Y(rotation_matrix, rotation_matrix, transform.rotation[1] * M_PI / 180.0);
            mat4x4_rotate_X(rotation_matrix, rotation_matrix, transform.rotation[0] * M_PI / 180.0);

            vec4 rotated_bullet_veolcity;
            mat4x4_mul_vec4(rotated_bullet_veolcity, rotation_matrix, bullet_velocity);

            bullet_rigid_body.velocity[0] = rotated_bullet_veolcity[0];
            bullet_rigid_body.velocity[1] = rotated_bullet_veolcity[1];
            bullet_rigid_body.velocity[2] = rotated_bullet_veolcity[2];

            m_entity_manager->SetEntityState(bullet_id, EntityState::ACTIVE);
            m_entity_manager->SetEntitySignature(bullet_id, PHYSICS_SYSTEM_SIGNATURE | COLLISION_SYSTEM_SIGNATURE);
        }

        // Handle player movement    
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

        if(transform.position[0] < -15) transform.position[0] = -15;
        if(transform.position[0] > 15) transform.position[0] = 15;
    }
    else if(player_input.state == PlayerState::GAMEOVER)
    {
        if(m_input_map.IsPressed(GLFW_KEY_SPACE))
        {
            Message message;
            message.message_type = MessageType::ZOOM;
            message.message_data = 0;
            m_zoom_on = false;
            m_message_bus.PostMessage(message);
            uint32_t crosshair_entity_id = m_entity_manager->GetEntityId("crosshair");
            m_entity_manager->SetEntityState(crosshair_entity_id, EntityState::INACTIVE);

            message.message_type = MessageType::XRAY;
            message.message_data = 0;
            m_xray_on = false;
            m_message_bus.PostMessage(message);

            m_entity_manager->SetEntityState(win_entity_id, EntityState::INACTIVE);
            m_entity_manager->SetEntityState(lose_entity_id, EntityState::INACTIVE);
            player_input.state = PlayerState::RUNNING;
            player_input.score = 0;
            player_input.timer = 60;

            RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
            Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
            transform.position[0] = 0;
            transform.position[1] = 1;
            transform.position[2] = 0;
            transform.rotation[0] = 0;
            transform.rotation[1] = 0;
            transform.rotation[2] = 0;

            message.message_type = MessageType::RESTART;
            m_message_bus.PostMessage(message);
        }
    }
    
}