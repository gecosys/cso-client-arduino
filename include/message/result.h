#ifndef _MESSAGE_RESULT_H_
#define _MESSAGE_RESULT_H_

#include <stdint.h>

template <class T>
struct Result {
    uint8_t errorCode;
    T data;
};

#endif
