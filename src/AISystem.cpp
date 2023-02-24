#include "AISystem.hpp"


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
        uint32_t enemy_id = message.message_data >> 16;
        RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(enemy_id);
        rigid_body.velocity[0] = -rigid_body.velocity[0];
        Texture& texture = m_component_manager->GetComponent<Texture>(enemy_id);
        if(rigid_body.velocity[0] > 0)
        {
            texture.position[0] = 0;
        }
        else
        {
            texture.position[0] = 128;
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