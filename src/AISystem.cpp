#include "AISystem.hpp"

#include <cstdio>


AISystem::AISystem(MessageBus& message_bus) : 
    System(message_bus, AI_SYSTEM_SIGNATURE)
{
}

AISystem::~AISystem()
{
}

void AISystem::HandleMessage(Message message)
{
    if(message.message_type == MessageType::COLLISION)
    {
        
        uint32_t entity_1_id = message.message_data >> 16;
        uint32_t entity_2_id = message.message_data & 0x0000FFFF;

        char* entity_1_tag = m_entity_manager->GetEntityTag(entity_1_id);
        char* entity_2_tag = m_entity_manager->GetEntityTag(entity_2_id);

        if(strcmp(entity_1_tag, "enemy") == 0)
        {
            uint32_t enemy_id = entity_1_id;
            RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(enemy_id);
            AIData& ai_data = m_component_manager->GetComponent<AIData>(enemy_id);
            ai_data.speed = -ai_data.speed;
            rigid_body.velocity[0] = ai_data.speed;
            Texture& texture = m_component_manager->GetComponent<Texture>(enemy_id);
            if(ai_data.speed > 0)
            {
                texture.position[1] = 0;
            }
            else
            {
                texture.position[1] = 128;
            }
        }
        else if(strncmp(entity_1_tag, "bullet", 6) == 0)
        {
            uint32_t bullet_id = m_entity_manager->GetEntityId(entity_1_tag);
            m_entity_manager->SetEntityState(bullet_id, EntityState::INACTIVE);
        }
    }
}

void AISystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Animation& animation = m_component_manager->GetComponent<Animation>(entity_id);
    Texture& texture = m_component_manager->GetComponent<Texture>(entity_id);

    animation.counter += delta_time;
    if(animation.counter > animation.speed)
    {
        animation.current_frame++;
        texture.position[0] += 64;
        if(animation.current_frame >= animation.num_frames)
        {
            animation.current_frame = 0;
            texture.position[0] = 0;
        }
        animation.counter = 0;
    }

}