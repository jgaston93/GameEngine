#ifndef PHYSICS_SYSTEM_HPP
#define PHYSICS_SYSTEM_HPP

#include "System.hpp"
#include "Signatures.hpp"

class PhysicsSystem : public System
{
    public:
    PhysicsSystem(MessageBus& message_bus);
    ~PhysicsSystem();

    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);

    private:
};

#endif // PHYSICS_SYSTEM_HPP