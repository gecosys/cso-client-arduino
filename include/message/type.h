#ifndef _MESSAGE_TYPE_H_
#define _MESSAGE_TYPE_H_

#include <stdint.h>

enum MessageType: uint8_t {
    Activation = 0x02,
    Single = 0x03,
    Group = 0x04,
    SingleCached = 0x05,
    GroupCached = 0x06,
    Done = 0x07
};

#endif
