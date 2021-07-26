#include <cstring>
#include "message/ticket.h"
#include "message/define.h"
#include "error/package/message_err.h"

Ticket::Ticket() noexcept 
    : id{},
      token{ LENGTH_TICKET_TOKEN } {}

Ticket::~Ticket() noexcept {}

uint16_t Ticket::getID() noexcept {
    return this->id;
}

const Array<uint8_t>& Ticket::getToken() noexcept {
    return this->token;
}

std::tuple<Error::Code, std::unique_ptr<Ticket>> Ticket::parseBytes(const Array<uint8_t>& data) noexcept {
    if (data.length() == 2 + LENGTH_TICKET_TOKEN) {
        std::unique_ptr<Ticket> ticket{ new Ticket{} };
        ticket->id = ((uint16_t)data[1] << 8U) | data[0];
        memcpy(ticket->token.get(), data.get() + 2, LENGTH_TICKET_TOKEN);
        return std::make_tuple(Error::Code::Nil, std::move(ticket));
    }
    return std::make_tuple(
        Error::buildCode(
            MessageErr::ID,
            MessageErr::Func::Ticket_ParseBytes,
            MessageErr::Reason::Ticket_InvalidTicketSize
        ),
        nullptr
    );
}

std::tuple<Error::Code, Array<uint8_t>> Ticket::buildBytes(uint16_t id, const Array<uint8_t>& token) noexcept {
    if (token.length() == LENGTH_TICKET_TOKEN) {
        uint8_t* buf = new uint8_t[2 + LENGTH_TICKET_TOKEN];
        buf[0] = (uint8_t)id;
        buf[1] = (uint8_t)(id >> 8U);
        memcpy(buf + 2, token.get(), LENGTH_TICKET_TOKEN);
        return std::make_tuple(Error::Code::Nil, Array<uint8_t>{ buf, 2 + LENGTH_TICKET_TOKEN });
    }
    return std::make_tuple(
        Error::buildCode(
            MessageErr::ID,
            MessageErr::Func::Ticket_ParseBytes,
            MessageErr::Reason::Ticket_InvalidTokenSize
        ),
        Array<uint8_t>{}
    );
}
