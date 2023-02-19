#ifndef COMPONENT_POOL_HPP
#define COMPONENT_POOL_HPP

#include <stdint.h>
#include <string.h>

template <typename T>
class ComponentPool
{
    public:
    ComponentPool(uint32_t num_entities);
    ~ComponentPool();

    void AddComponent(uint32_t entity_id, T component);
    T& GetComponent(uint32_t entity_id);

    private:
    const uint32_t m_num_components;
    T* m_component_data;
    
};


template <typename T>
ComponentPool<T>::ComponentPool(uint32_t num_entities) : m_num_components(num_entities),
                                                         m_component_data(new T[num_entities])
{
    memset(m_component_data, 0, num_entities * sizeof(T));
}

template <typename T>
ComponentPool<T>::~ComponentPool()
{
    delete[] m_component_data;
}

template <typename T>
void ComponentPool<T>::AddComponent(uint32_t entity_id, T component)
{
    m_component_data[entity_id] = component;
}

template <typename T>
T& ComponentPool<T>::GetComponent(uint32_t entity_id)
{
    return m_component_data[entity_id];
}

#endif // COMPONENT_POOL_HPP