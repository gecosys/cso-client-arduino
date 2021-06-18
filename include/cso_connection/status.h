#ifndef _CSO_CONNECTION_STATUS_H_
#define _CSO_CONNECTION_STATUS_H_

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

#endif // _CSO_CONNECTION_STATUS_H_