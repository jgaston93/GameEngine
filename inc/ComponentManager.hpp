#ifndef COMPONENT_MANAGER_HPP
#define COMPONENT_MANAGER_HPP

#include "ComponentPool.hpp"
#include "Transform.hpp"
#include "Texture.hpp"
#include "RigidBody.hpp"
#include "PlayerInput.hpp"
#include "BoundingBox.hpp"
#include "Quad.hpp"
#include "Animation.hpp"
#include "LabelTexture.hpp"
#include "Timer.hpp"
#include "Bounds.hpp"

class ComponentManager
{
    public:
    ComponentManager(uint32_t num_entities);
    ~ComponentManager();

    template <typename T>
    void AddComponent(uint32_t entity_id, T component);

    template <typename T>
    T& GetComponent(uint32_t entity_id);

    private:
    ComponentPool<Transform> m_transform_pool;
    ComponentPool<Texture> m_texture_pool;
    ComponentPool<RigidBody> m_rigid_body_pool;
    ComponentPool<PlayerInput> m_player_input_pool;
    ComponentPool<BoundingBox> m_bounding_box_pool;
    ComponentPool<Quad> m_quad_pool;
    ComponentPool<Animation> m_animation_pool;
    ComponentPool<LabelTexture> m_label_texture_pool;
    ComponentPool<Timer> m_timer_pool;
    ComponentPool<Bounds> m_bounds_pool;
};

#endif // COMPONENT_MANAGER_HPP