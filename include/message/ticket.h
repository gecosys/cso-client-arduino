#ifndef MESSAGE_TICKET_H
#define MESSAGE_TICKET_H

#include <tuple>
#include <memory>
#include <cstdint>
#include "error/error.h"
#include "entity/array.hpp"

class Ticket {
private:
    uint16_t id;
    Array<uint8_t> token;

public:
    Ticket() noexcept;
    ~Ticket() noexcept;

    uint16_t getID() noexcept;
    const Array<uint8_t>& getToken() noexcept;

    static std::tuple<Error, std::unique_ptr<Ticket>> parseBytes(const Array<uint8_t>& data) noexcept;
    static std::tuple<Error, Array<uint8_t>> buildBytes(uint16_t id, const Array<uint8_t>& token) noexcept;
};

#endif // !MESSAGE_TICKET_H