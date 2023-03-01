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
            RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_2_id);
            AIData& ai_data = m_component_manager->GetComponent<AIData>(entity_2_id);
            rigid_body.velocity[0] = 0;
            ai_data.alive = false;
        }
    }
    if(message.message_type == MessageType::RESTART)
    {
        uint32_t num_entites = m_entity_manager->GetNumEntities();
        for(uint32_t i = 0; i < num_entites; i++)
        {
            if(m_entity_manager->GetEntitySignature(i) & AI_SYSTEM_SIGNATURE)
            {
                Transform& transform = m_component_manager->GetComponent<Transform>(i);
                AIData& ai_data = m_component_manager->GetComponent<AIData>(i);
                RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(i);
                transform.position[0] = ai_data.position[0];
                transform.position[1] = ai_data.position[1];
                transform.position[2] = ai_data.position[2];
                transform.rotation[0] = ai_data.rotation[0];
                transform.rotation[1] = ai_data.rotation[1];
                transform.rotation[2] = ai_data.rotation[2];
                rigid_body.velocity[0] = ai_data.speed;
                ai_data.alive = true;
            }
        }
    }
}

void AISystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);
    Animation& animation = m_component_manager->GetComponent<Animation>(entity_id);
    Texture& texture = m_component_manager->GetComponent<Texture>(entity_id);
    AIData& ai_data = m_component_manager->GetComponent<AIData>(entity_id);

    if(ai_data.alive)
    {
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
    else if(transform.rotation[0] > -90)
    {
        transform.rotation[0] -= 100 * delta_time;
        transform.position[1] = ai_data.initial_height - 0.70 * -(1 - ((90 - transform.rotation[0]) / 90));
    }

}