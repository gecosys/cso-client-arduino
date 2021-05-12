#include <message/ticket.h>
#include <message/define.h>
#include <string.h>

uint16_t Ticket::getID() {
    return this->id;
}

uint8_t *Ticket::getToken() {
    return this->token;
}

Result<Ticket *> Ticket::parseBytes(uint8_t *buffer, uint8_t sizeBuffer) {
    Result<Ticket *> result;
    if (sizeBuffer != LENGTH_TICKET) {
        result.errorCode = ERROR_CODE_INVALID_BYTES;
        return result;
    }
    Ticket *ticket = new Ticket();
    ticket->id = ((uint16_t)buffer[1] << 8) | buffer[0];
    memcpy(ticket->token, buffer+2, 32);

    result.data = ticket;
    result.errorCode = SUCCESS;
    return result;
}

Result<uint8_t *> Ticket::buildBytes(uint16_t id, uint8_t token[32]) {
    uint8_t *buffer = new uint8_t[LENGTH_TICKET];
    buffer[0] = (uint8_t)id;
    buffer[1] = (uint8_t)(id >> 8);
    memcpy(buffer+2, token, 32);

    Result<uint8_t *> result;
    result.data = buffer;
    result.errorCode = SUCCESS;
    return result;
}
