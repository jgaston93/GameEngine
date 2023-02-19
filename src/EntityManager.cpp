#include <cstring>

#include "EntityManager.hpp"

EntityManager::EntityManager(uint32_t num_entities) : m_num_entities(num_entities),
                                                    m_entity_signatures(new uint32_t[num_entities]),
                                                    m_entity_tags(new char*[num_entities]),
                                                    m_entity_states(new EntityState[num_entities])
{
    memset(m_entity_signatures, 0, num_entities * sizeof(uint32_t));
    memset(m_entity_states, 0, num_entities * sizeof(EntityState));
    for(uint32_t i = 0; i < num_entities; i++)
    {
        m_entity_tags[i] = new char[tag_length];
        memset(m_entity_tags[i], 0, tag_length);
    }
}

EntityManager::~EntityManager()
{
    delete[] m_entity_signatures;
    delete[] m_entity_states;
    for(uint32_t i = 0; i < m_num_entities; i++)
    {
        delete[] m_entity_tags[i];
    }
    delete[] m_entity_tags;
}

void EntityManager::SetEntitySignature(uint32_t entity_id, uint32_t signature)
{
    m_entity_signatures[entity_id] = signature;
}

void EntityManager::SetEntityState(uint32_t entity_id, EntityState state)
{
    m_entity_states[entity_id] = state;
}

void EntityManager::SetEntityTag(uint32_t entity_id, char* tag)
{
    strncpy(m_entity_tags[entity_id], tag, tag_length);
}

uint32_t EntityManager::GetNumEntities()
{
    return m_num_entities;
}

uint32_t EntityManager::GetEntitySignature(uint32_t entity_id)
{
    return m_entity_signatures[entity_id];
}

EntityState EntityManager::GetEntityState(uint32_t entity_id)
{
    return m_entity_states[entity_id];
}

char* EntityManager::GetEntityTag(uint32_t entity_id)
{
    return m_entity_tags[entity_id];
}

uint32_t EntityManager::GetEntityId(char* entity_tag)
{
    uint32_t entity_id = 0;
    for(uint32_t i = 0; i < m_num_entities; i++)
    {
        if(strcmp(m_entity_tags[i], entity_tag) == 0)
        {
            entity_id = i;
        }
    }
    return entity_id;
}
