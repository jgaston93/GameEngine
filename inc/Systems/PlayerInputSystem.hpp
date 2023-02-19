#ifndef PLAYER_INPUT_SYSTEM_HPP
#define PLAYER_INPUT_SYSTEM_HPP

#include "System.hpp"
#include "InputMap.hpp"
#include "Signatures.hpp"

class PlayerInputSystem : public System
{
    public:
    PlayerInputSystem(MessageBus& message_bus, InputMap& input_map);
    ~PlayerInputSystem();
    
    void HandleMessage(Message message);
    void HandleEntity(uint32_t entity_id, float delta_time);

    private:
    InputMap& m_input_map;
};

#endif // PLAYER_INPUT_SYSTEM_HPP