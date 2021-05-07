#ifndef _UTILS_CODE_H_
#define _UTILS_CODE_H_

#include <cstdint>

class Error {
public:
    enum Code : uint8_t {
        Nil = 0,
        Full,
        Empty,
        Other,
        Verify,
        Encrypt,
        Decrypt,
        HttpError,
        CanNotSync,
        NotEnoughMem,
        NotConnectServer,
    };
};

#endif //_UTILS_CODE_H_