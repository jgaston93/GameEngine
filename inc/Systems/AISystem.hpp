#ifndef AI_SYSTEM_HPP
#define AI_SYSTEM_HPP

#include "System.hpp"
#include "Signatures.hpp"

class AISystem : public System
{
    public:
    AISystem(MessageBus& message_bus);
    ~AISystem();

    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);

    private:
};

#endif // AI_SYSTEM_HPP