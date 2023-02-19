#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

#include <stdint.h>

enum EntityState
{
    INACTIVE,
    ACTIVE,
};

const uint32_t tag_length = 64; // includes null terminator

class EntityManager
{
    public:
    EntityManager(uint32_t num_entities);
    ~EntityManager();
    
    void SetEntitySignature(uint32_t entity_id, uint32_t signature);
    void SetEntityState(uint32_t entity_id, EntityState state);
    void SetEntityTag(uint32_t entity_id, char* tag);
    uint32_t GetNumEntities();
    uint32_t GetEntitySignature(uint32_t entity_id);
    EntityState GetEntityState(uint32_t entity_id);
    uint32_t GetEntityId(char* entity_tag);
    char* GetEntityTag(uint32_t entity_id);

    private:
    const uint32_t m_num_entities;
    uint32_t* m_entity_signatures;
    char** m_entity_tags;
    EntityState* m_entity_states;


};

#endif // ENTITY_MANAGER_HPP