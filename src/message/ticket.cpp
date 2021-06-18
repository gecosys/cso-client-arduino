#include <new>
#include <cstring>
#include "message/ticket.h"
#include "message/define.h"

uint16_t Ticket::getID() noexcept {
    return this->id;
}

uint8_t *Ticket::getToken() noexcept {
    return this->token;
}

Result<Ticket*> Ticket::parseBytes(uint8_t* buffer, uint8_t sizeBuffer) noexcept {
    Result<Ticket*> result;
    if (sizeBuffer != LENGTH_TICKET) {
        result.errorCode = Error::Message_InvalidBytes;
        return result;
    }
    Ticket* ticket = new Ticket();
    if (ticket == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
    ticket->id = ((uint16_t)buffer[1] << 8U) | buffer[0];
    memcpy(ticket->token, buffer + 2, 32);

    result.data = ticket;
    result.errorCode = Error::Nil;
    return result;
}

Result<uint8_t*> Ticket::buildBytes(uint16_t id, uint8_t token[32]) noexcept {
    Result<uint8_t*> result;
    uint8_t* buffer = new (std::nothrow) uint8_t[LENGTH_TICKET];
    if (buffer == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
    buffer[0] = (uint8_t)id;
    buffer[1] = (uint8_t)(id >> 8U);
    memcpy(buffer + 2, token, 32);

    result.data = buffer;
    result.errorCode = Error::Nil;
    return result;
}
