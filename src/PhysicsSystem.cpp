#include <stdio.h>

#include "PhysicsSystem.hpp"


PhysicsSystem::PhysicsSystem(MessageBus& message_bus) : 
    System(message_bus, PHYSICS_SYSTEM_SIGNATURE)
{

}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::HandleMessage(Message message)
{
    
}

void PhysicsSystem::HandleEntity(uint32_t entity_id, float delta_time)
{
    RigidBody& rigid_body = m_component_manager->GetComponent<RigidBody>(entity_id);
    Transform& transform = m_component_manager->GetComponent<Transform>(entity_id);

    rigid_body.velocity[0] += rigid_body.acceleration[0];
    rigid_body.velocity[1] += rigid_body.acceleration[1];
    rigid_body.velocity[2] += rigid_body.acceleration[2];

    float delta_time_x = delta_time;
    float delta_time_y = delta_time;
    float delta_time_z = delta_time;
    
    if(m_entity_manager->GetEntitySignature(entity_id) & COLLISION_SYSTEM_SIGNATURE)
    {
        float intended_x_position = transform.position[0] + rigid_body.velocity[0] * delta_time;
        float intended_y_position = transform.position[1] + rigid_body.velocity[1] * delta_time;

        BoundingBox& bounding_box = m_component_manager->GetComponent<BoundingBox>(entity_id);
        float half_width = bounding_box.extent[0] / 2;
        float half_height = bounding_box.extent[1] / 2;
        float min_x = transform.position[0] - half_width;
        float max_x = transform.position[0] + half_width;
        float min_y = transform.position[1] - half_height;
        float max_y = transform.position[1] + half_height;
        float intended_min_x = intended_x_position - half_width;
        float intended_max_x = intended_x_position + half_width;
        float intended_min_y = intended_y_position - half_height;
        float intended_max_y = intended_y_position + half_height;

        uint32_t num_entities = m_entity_manager->GetNumEntities();
        for(uint32_t other_entity_id = 0; other_entity_id < num_entities; other_entity_id++)
        {
            if(other_entity_id != entity_id && m_entity_manager->GetEntitySignature(other_entity_id) & COLLISION_SYSTEM_SIGNATURE)
            {
                BoundingBox& other_bounding_box = m_component_manager->GetComponent<BoundingBox>(other_entity_id);
                Transform& other_transform = m_component_manager->GetComponent<Transform>(other_entity_id);
                float other_half_width = other_bounding_box.extent[0] / 2;
                float other_half_height = other_bounding_box.extent[1] / 2;
                float other_min_x = other_transform.position[0] - other_half_width;
                float other_max_x = other_transform.position[0] + other_half_width;
                float other_min_y = other_transform.position[1] - other_half_height;
                float other_max_y = other_transform.position[1] + other_half_height;

                bool collision_x = max_x >= other_min_x && other_max_x >= min_x;
                bool collision_y = max_y >= other_min_y && other_max_y >= min_y;
                bool intended_collision_x = intended_max_x >= other_min_x && other_max_x >= intended_min_x;
                bool intended_collision_y = intended_max_y >= other_min_y && other_max_y >= intended_min_y;

                if(intended_collision_x && collision_y)
                {
                    float dx = 0;
                    if(min_x < other_min_x)
                    {
                        dx = other_min_x - max_x;
                    }
                    else if(min_x > other_min_x)
                    {
                        dx = min_x - other_max_x;
                    }
                    float x_time = abs(dx / rigid_body.velocity[0]);
                    if(x_time < delta_time_x)
                    {
                        delta_time_x = x_time;
                    }
                }
                if(collision_x && intended_collision_y)
                {
                    float dy = 0;
                    if(min_y < other_min_y)
                    {
                        dy = other_min_y - max_y;
                    }
                    else if(min_y > other_min_y)
                    {
                        dy = min_y - other_max_y;
                    }
                    float y_time = abs(dy / rigid_body.velocity[1]);
                    if(y_time < delta_time_y)
                    {
                        delta_time_y = y_time;
                    }
                }
            }
        }
    }

    transform.position[0] += rigid_body.velocity[0] * delta_time_x;
    transform.position[1] += rigid_body.velocity[1] * delta_time_y;
    transform.position[2] += rigid_body.velocity[2] * delta_time_z;

    if(delta_time_x < delta_time)
    {
        rigid_body.acceleration[0] = 0;
        rigid_body.velocity[0] = 0;
    }
    if(delta_time_y < delta_time)
    {
        rigid_body.acceleration[1] = 0;
        rigid_body.velocity[1] = 0;
    }
    if(delta_time_z < delta_time)
    {
        rigid_body.acceleration[2] = 0;
        rigid_body.velocity[2] = 0;
    }
}