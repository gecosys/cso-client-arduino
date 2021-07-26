#include "message/readyticket.h"
#include "error/package/message_err.h"

ReadyTicket::ReadyTicket() noexcept 
    : isReady{ false },
      maskRead{ 0 },
      idxRead{ 0 },
      idxWrite{ 0 } {}

ReadyTicket::~ReadyTicket() noexcept {}

bool ReadyTicket::getIsReady() noexcept {
    return this->isReady;
}

uint64_t ReadyTicket::getIdxRead() noexcept {
    return this->idxRead;
}

uint32_t ReadyTicket::getMaskRead() noexcept {
    return this->maskRead;
}

uint64_t ReadyTicket::getIdxWrite() noexcept {
    return this->idxWrite;
}

// ParseBytes converts bytes to ReadyTicket
// Flag is_ready: 1 byte
// Idx Read: 8 bytes
// Mark Read: 4 bytes
// Idx Write: 8 bytes
std::tuple<Error::Code, std::unique_ptr<ReadyTicket>> ReadyTicket::parseBytes(const Array<uint8_t>& data) noexcept {
    if (data.length() != 21) {
        return std::make_tuple(
             Error::buildCode(
                MessageErr::ID,
                MessageErr::Func::ReadyTicket_ParseBytes,
                MessageErr::Reason::ReadyTicket_InvalidSize
            ),
            nullptr
        );
    }

    std::unique_ptr<ReadyTicket> ticket{ new ReadyTicket() };
    ticket->isReady = data[0] == 1;
    ticket->idxRead = ((uint64_t)data[8] << 56U) |
                      ((uint64_t)data[7] << 48U) |
                      ((uint64_t)data[6] << 40U) |
                      ((uint64_t)data[5] << 32U) |
                      ((uint64_t)data[4] << 24U) |
                      ((uint64_t)data[3] << 16U) |
                      ((uint64_t)data[2] << 8U) | data[1];

    ticket->maskRead = ((uint32_t)data[12] << 24U) |
                       ((uint32_t)data[11] << 16U) |
                       ((uint32_t)data[10]) << 8U | data[9];

    ticket->idxWrite = ((uint64_t)data[20] << 56U) |
                       ((uint64_t)data[19] << 48U) |
                       ((uint64_t)data[18] << 40U) |
                       ((uint64_t)data[17] << 32U) |
                       ((uint64_t)data[16] << 24U) |
                       ((uint64_t)data[15] << 16U) |
                       ((uint64_t)data[14] << 8U) | data[13];
    return std::make_tuple(Error::Code::Nil, std::move(ticket));
}
