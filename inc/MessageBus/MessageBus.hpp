#ifndef MESSAGE_BUS_HPP
#define MESSAGE_BUS_HPP

#include "System.hpp"
#include "Message.hpp"

class System;

class MessageBus
{
    public:
    MessageBus(uint32_t max_num_messages, uint32_t max_num_systems);
    ~MessageBus();
    void RegisterSystem(System* system);
    void Update();
    void PostMessage(Message message);

    private:
    const uint32_t m_max_num_messages;
    uint32_t m_num_messages;
    uint32_t m_message_queue_index;
    Message* m_message_queue;

    const uint32_t m_max_num_systems;
    uint32_t m_num_systems;
    System** m_systems;

};

#endif // MESSAGE_BUS_HPP