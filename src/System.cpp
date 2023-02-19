#include "System.hpp"


System::System(MessageBus& message_bus, uint32_t system_signature) : m_message_bus(message_bus), m_system_signature(system_signature)
{
    message_bus.RegisterSystem(this);
}

void System::SetEntityManager(EntityManager* entity_manager)
{
    m_entity_manager = entity_manager;
}

void System::SetComponentManager(ComponentManager* component_manager)
{
    m_component_manager = component_manager;
}

void System::Update(float delta_time)
{
    uint32_t num_entities = m_entity_manager->GetNumEntities();
    
    for(uint32_t i = 0; i < num_entities; i++)
    {
        if(m_entity_manager->GetEntityState(i) == EntityState::ACTIVE &&
            (m_entity_manager->GetEntitySignature(i) & m_system_signature))
        {
            HandleEntity(i, delta_time);
        }
    }
}
