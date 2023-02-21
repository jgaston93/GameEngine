#include "ComponentManager.hpp"


ComponentManager::ComponentManager(uint32_t num_entities) : m_transform_pool(num_entities),
                                                            m_rigid_body_pool(num_entities),
                                                            m_texture_pool(num_entities),
                                                            m_player_input_pool(num_entities),
                                                            m_bounding_box_pool(num_entities),
                                                            m_quad_pool(num_entities),
                                                            m_animation_pool(num_entities),
                                                            m_label_texture_pool(num_entities),
                                                            m_timer_pool(num_entities),
                                                            m_bounds_pool(num_entities),
                                                            m_label_pool(num_entities)
{

}

ComponentManager::~ComponentManager()
{

}

template <>
void ComponentManager::AddComponent<Transform>(uint32_t entity_id, Transform component)
{
    m_transform_pool.AddComponent(entity_id, component);
}

template <>
Transform& ComponentManager::GetComponent<Transform>(uint32_t entity_id)
{
    return m_transform_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<RigidBody>(uint32_t entity_id, RigidBody component)
{
    m_rigid_body_pool.AddComponent(entity_id, component);
}

template <>
RigidBody& ComponentManager::GetComponent<RigidBody>(uint32_t entity_id)
{
    return m_rigid_body_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Texture>(uint32_t entity_id, Texture component)
{
    m_texture_pool.AddComponent(entity_id, component);
}

template <>
Texture& ComponentManager::GetComponent<Texture>(uint32_t entity_id)
{
    return m_texture_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<PlayerInput>(uint32_t entity_id, PlayerInput component)
{
    m_player_input_pool.AddComponent(entity_id, component);
}

template <>
PlayerInput& ComponentManager::GetComponent<PlayerInput>(uint32_t entity_id)
{
    return m_player_input_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<BoundingBox>(uint32_t entity_id, BoundingBox component)
{
    m_bounding_box_pool.AddComponent(entity_id, component);
}

template <>
BoundingBox& ComponentManager::GetComponent<BoundingBox>(uint32_t entity_id)
{
    return m_bounding_box_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Quad>(uint32_t entity_id, Quad component)
{
    m_quad_pool.AddComponent(entity_id, component);
}

template <>
Quad& ComponentManager::GetComponent<Quad>(uint32_t entity_id)
{
    return m_quad_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Animation>(uint32_t entity_id, Animation component)
{
    m_animation_pool.AddComponent(entity_id, component);
}

template <>
Animation& ComponentManager::GetComponent<Animation>(uint32_t entity_id)
{
    return m_animation_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<LabelTexture>(uint32_t entity_id, LabelTexture component)
{
    m_label_texture_pool.AddComponent(entity_id, component);
}

template <>
LabelTexture& ComponentManager::GetComponent<LabelTexture>(uint32_t entity_id)
{
    return m_label_texture_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Timer>(uint32_t entity_id, Timer component)
{
    m_timer_pool.AddComponent(entity_id, component);
}

template <>
Timer& ComponentManager::GetComponent<Timer>(uint32_t entity_id)
{
    return m_timer_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Bounds>(uint32_t entity_id, Bounds component)
{
    m_bounds_pool.AddComponent(entity_id, component);
}

template <>
Bounds& ComponentManager::GetComponent<Bounds>(uint32_t entity_id)
{
    return m_bounds_pool.GetComponent(entity_id);
}

template <>
void ComponentManager::AddComponent<Label>(uint32_t entity_id, Label component)
{
    m_label_pool.AddComponent(entity_id, component);
}

template <>
Label& ComponentManager::GetComponent<Label>(uint32_t entity_id)
{
    return m_label_pool.GetComponent(entity_id);
}