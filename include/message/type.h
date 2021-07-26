#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

#include <stdint.h>

enum MessageType: uint8_t {
    Activation   = 2U,
    Single       = 3U,
    Group        = 4U,
    SingleCached = 5U,
    GroupCached  = 6U,
    Done         = 7U
};

#endif // !MESSAGE_TYPE_H
