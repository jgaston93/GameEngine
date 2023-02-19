#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "MessageBus.hpp"
#include "EntityManager.hpp"
#include "ComponentManager.hpp"

class MessageBus;
class Message;

class System
{
    public:
        System(MessageBus& message_bus, uint32_t system_signature);
        void SetEntityManager(EntityManager* entity_manager);
        void SetComponentManager(ComponentManager* component_manager);
        virtual void Update(float delta_time);
        virtual void HandleMessage(Message message) = 0;
        virtual void HandleEntity(uint32_t entity_id, float delata_time) = 0;

    protected:
        MessageBus& m_message_bus;
        EntityManager* m_entity_manager;
        ComponentManager* m_component_manager;
        const uint32_t m_system_signature;
        
};

#endif // SYSTEM_HPP