#ifndef _MESSAGE_TICKET_H_
#define _MESSAGE_TICKET_H_

#include <cstdint>
#include "message/result.h"

class Ticket {
private:
    uint16_t id;
    uint8_t token[32];

public:
    uint16_t getID() noexcept;
    uint8_t* getToken() noexcept;

    static Result<Ticket*> parseBytes(uint8_t* buffer, uint8_t sizeBuffer) noexcept;
    static Result<uint8_t*> buildBytes(uint16_t id, uint8_t token[32]) noexcept;
};

#endif
