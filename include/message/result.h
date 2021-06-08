#ifndef _MESSAGE_RESULT_H_
#define _MESSAGE_RESULT_H_

#include <cstdint>
#include "error/error_code.h"

template <class T>
struct Result {
    Error::Code errorCode;
    T data;
};

#endif
