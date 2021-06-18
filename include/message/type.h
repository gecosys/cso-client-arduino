#ifndef _MESSAGE_TYPE_H_
#define _MESSAGE_TYPE_H_

#include <stdint.h>

enum MessageType: uint8_t {
    Activation   = 0x02U,
    Single       = 0x03U,
    Group        = 0x04U,
    SingleCached = 0x05U,
    GroupCached  = 0x06U,
    Done         = 0x07U
};

#endif
