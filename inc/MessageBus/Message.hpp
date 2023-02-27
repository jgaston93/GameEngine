#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <stdint.h>

enum MessageType
{
    QUIT,
    KEYPRESS,
    KEYRELEASE,
    COLLISION,
    ZOOM,
    XRAY
};

struct Message
{
    MessageType message_type;
    uint32_t message_data;
};

#endif // MESSAGE_HPP