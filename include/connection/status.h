#ifndef _CONNECTION_STATUS_H_
#define _CONNECTION_STATUS_H_

#include <cstdint>

class Status {
public:
    enum Code : uint8_t {
        Prepare = 0,
        Connecting,
        Connected,
        Disconnected,
    };
};

#endif // _CONNECTION_STATUS_H_