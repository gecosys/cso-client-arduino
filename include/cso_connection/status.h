#ifndef CSO_CONNECTION_STATUS_H
#define CSO_CONNECTION_STATUS_H

#include <cstdint>

class Status {
public:
    enum Code : uint8_t {
        Connected    = 1U,
        Disconnected = 2U,
    };
};

#endif // !CSO_CONNECTION_STATUS_H