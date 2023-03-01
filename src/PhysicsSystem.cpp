#include <stdio.h>
#include <limits>
#include <algorithm>

#include "PhysicsSystem.hpp"


void GetInvEntryExit(float v, float min1, float max1, float min2, float max2, float& inv_entry, float& inv_exit)
{
    if(v > 0)
    {
        inv_entry = min2 - max1;
        inv_exit = max2 - min1;
    }
    else
    {
        inv_entry = max2 - min1;
        inv_exit = min2 - max1;
    }
}

void GetEntryExit(float v, float inv_entry, float inv_exit, float& entry, float& exit)
{
    if(v == 0)
    {
        entry = -std::numeric_limits<float>::infinity();
        exit = std::numeric_limits<float>::infinity();
    }
    else
    {
        entry = inv_entry / v;
        exit = inv_exit / v;
    }
}


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

    float move_time = delta_time;
    float move_time_x = delta_time;
    float move_time_y = delta_time;
    float move_time_z = delta_time;
    int32_t collision_entity_id = -1;
    vec3 normal = {0, 0, 0};
    
    if(m_entity_manager->GetEntitySignature(entity_id) & COLLISION_SYSTEM_SIGNATURE)
    {
        float intended_x_position = transform.position[0] + rigid_body.velocity[0] * delta_time;
        float intended_y_position = transform.position[1] + rigid_body.velocity[1] * delta_time;
        float intended_z_position = transform.position[2] + rigid_body.velocity[2] * delta_time;

        BoundingBox& bounding_box = m_component_manager->GetComponent<BoundingBox>(entity_id);
        float half_width = bounding_box.extent[0] / 2;
        float half_height = bounding_box.extent[1] / 2;
        float half_depth = bounding_box.extent[2] / 2;
        float min_x = transform.position[0] - half_width;
        float max_x = transform.position[0] + half_width;
        float min_y = transform.position[1] - half_height;
        float max_y = transform.position[1] + half_height;
        float min_z = transform.position[2] - half_depth;
        float max_z = transform.position[2] + half_depth;
        float intended_min_x = intended_x_position - half_width;
        float intended_max_x = intended_x_position + half_width;
        float intended_min_y = intended_y_position - half_height;
        float intended_max_y = intended_y_position + half_height;
        float intended_min_z = intended_z_position - half_depth;
        float intended_max_z = intended_z_position + half_depth;

        // Calculate broadphase bounding box
        float bp_min_x = min_x;
        float bp_max_x = max_x;
        float bp_min_y = min_y;
        float bp_max_y = max_y;
        float bp_min_z = min_z;
        float bp_max_z = max_z;

        if(rigid_body.velocity[0] != 0)
        {
            bp_min_x = rigid_body.velocity[0] > 0 ? min_x : intended_min_x;
            bp_max_x = rigid_body.velocity[0] > 0 ? intended_max_x : min_x;
        }
        if(rigid_body.velocity[1] != 0)
        {
            bp_min_y = rigid_body.velocity[1] > 0 ? min_y : intended_min_y;
            bp_max_y = rigid_body.velocity[1] > 0 ? intended_max_y : min_y;
        }
        if(rigid_body.velocity[2] != 0)
        {
            bp_min_z = rigid_body.velocity[2] > 0 ? min_z : intended_min_z;
            bp_max_z = rigid_body.velocity[2] > 0 ? intended_max_z : min_z;
        }

        uint32_t num_entities = m_entity_manager->GetNumEntities();
        for(uint32_t other_entity_id = 0; other_entity_id < num_entities; other_entity_id++)
        {
            if(other_entity_id != entity_id && m_entity_manager->GetEntityState(other_entity_id) == EntityState::ACTIVE && m_entity_manager->GetEntitySignature(other_entity_id) & COLLISION_SYSTEM_SIGNATURE)
            {
                BoundingBox& other_bounding_box = m_component_manager->GetComponent<BoundingBox>(other_entity_id);
                Transform& other_transform = m_component_manager->GetComponent<Transform>(other_entity_id);
                float other_half_width = other_bounding_box.extent[0] / 2;
                float other_half_height = other_bounding_box.extent[1] / 2;
                float other_half_depth = other_bounding_box.extent[2] / 2;
                float other_min_x = other_transform.position[0] - other_half_width;
                float other_max_x = other_transform.position[0] + other_half_width;
                float other_min_y = other_transform.position[1] - other_half_height;
                float other_max_y = other_transform.position[1] + other_half_height;
                float other_min_z = other_transform.position[2] - other_half_depth;
                float other_max_z = other_transform.position[2] + other_half_depth;

                bool collision_x = bp_max_x >= other_min_x && other_max_x >= bp_min_x;
                bool collision_y = bp_max_y >= other_min_y && other_max_y >= bp_min_y;
                bool collision_z = bp_max_z >= other_min_z && other_max_z >= bp_min_z;


                if(collision_x && collision_y && collision_z)
                {
                    float xInvEntry, yInvEntry, zInvEntry;
                    float xInvExit, yInvExit, zInvExit;
                    float xEntry, yEntry, zEntry;
                    float xExit, yExit, zExit;
                    
                    GetInvEntryExit(rigid_body.velocity[0], min_x, max_x, other_min_x, other_max_x, xInvEntry, xInvExit);
                    GetInvEntryExit(rigid_body.velocity[1], min_y, max_y, other_min_y, other_max_y, yInvEntry, yInvExit);
                    GetInvEntryExit(rigid_body.velocity[2], min_z, max_z, other_min_z, other_max_z, zInvEntry, zInvExit);

                    GetEntryExit(rigid_body.velocity[0], xInvEntry, xInvExit, xEntry, xExit);
                    GetEntryExit(rigid_body.velocity[1], yInvEntry, yInvExit, yEntry, yExit);
                    GetEntryExit(rigid_body.velocity[2], zInvEntry, zInvExit, zEntry, zExit);

                    float entry_time = std::max(std::max(xEntry, yEntry), zEntry);
                    float exit_time = std::min(std::min(xExit, yExit), zExit);

                    if(!(entry_time > exit_time || (xEntry < 0.0f && yEntry < 0.0f && zEntry < 0.0f) || xEntry > 1.0f || yEntry > 1.0f || zEntry > 1.0f))
                    {
                        if(entry_time < move_time)
                        {
                            move_time = entry_time;
                            collision_entity_id = other_entity_id;
                        }
                        if(entry_time == xEntry)
                        {
                            if(xInvEntry < 0)
                            {
                                normal[0] = 1;
                            }
                            else if(xInvEntry > 0)
                            {
                                normal[0] = -1;
                            }
                        }
                        else if(entry_time == yEntry)
                        {
                            if(yInvEntry < 0)
                            {
                                normal[1] = 1;
                            }
                            else if(yInvEntry > 0)
                            {
                                normal[1] = -1;
                            }
                        }
                        if(entry_time == zEntry)
                        {
                            if(zInvEntry < 0)
                            {
                                normal[2] = 1;
                            }
                            else if(zInvEntry > 0)
                            {
                                normal[2] = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    if(move_time < delta_time)
    {

        float dotprod = (rigid_body.velocity[0] * normal[2] + rigid_body.velocity[1] * normal[1] + rigid_body.velocity[2] * normal[0]) * move_time;

        transform.position[0] += dotprod * normal[2];
        transform.position[1] += dotprod * normal[1];
        transform.position[2] += dotprod * normal[0];
        
        char* entity_1_tag = m_entity_manager->GetEntityTag(entity_id);
        char* entity_2_tag = m_entity_manager->GetEntityTag(collision_entity_id);
        if(strcmp(entity_1_tag, "enemy") == 0 && strcmp(entity_2_tag, "side_wall") == 0)
        {
            Message message;
            message.message_type = MessageType::COLLISION;
            message.message_data = (entity_id << 16) + collision_entity_id;
            m_message_bus.PostMessage(message);
        }
        
        if(strncmp(entity_1_tag, "bullet", 6) == 0 && strcmp(entity_2_tag, "enemy") == 0)
        {
            Message message;
            message.message_type = MessageType::COLLISION;
            message.message_data = (entity_id << 16) + collision_entity_id;
            m_message_bus.PostMessage(message);
        }
    }
    else
    {
        transform.position[0] += rigid_body.velocity[0] * delta_time;
        transform.position[1] += rigid_body.velocity[1] * delta_time;
        transform.position[2] += rigid_body.velocity[2] * delta_time;
    }
}