#ifndef ERROR_EXTERNAL_H
#define ERROR_EXTERNAL_H

#include <string>
#include <cstdint>

#define EXTERNAL_NID 4

class External {
public:
    enum ID : uint8_t {
        Nil		    = 0U,
        MbedTLS     = 1U,
        ArduinoJSON = 2U,
        HTTP        = 3U,
        Server      = 4U,
    };

public:
    static std::string getReason(uint8_t extID, int32_t reasonCode);
};

#endif // !ERROR_EXTERNAL_H