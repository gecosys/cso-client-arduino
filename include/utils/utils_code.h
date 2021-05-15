#ifndef _UTILS_CODE_H_
#define _UTILS_CODE_H_

#include <cstdint>

class Error {
public:
    enum Code : int8_t {
        Nil                 = 0,
        Full                = -1,
        Empty               = -2,
        Other               = -3,
        Build               = -4,
        Parse               = -5,
        Verify              = -6,
        Encrypt             = -7,
        Decrypt             = -8,
        HttpError           = -9,
        NotReady            = -10,
        NotEnoughMem        = -11,
        NotConnectServer    = -12,
        RespEmpty           = -13,
    };
};

#endif //_UTILS_CODE_H_