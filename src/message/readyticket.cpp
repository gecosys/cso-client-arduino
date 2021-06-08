#include <new>
#include "message/readyticket.h"


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
Result<ReadyTicket*> ReadyTicket::parseBytes(uint8_t* buffer, uint8_t sizeBuffer) noexcept {
    Result<ReadyTicket*> result;
    if (sizeBuffer != 21) {
        result.errorCode = Error::Message_InvalidBytes;
        return result;
    }

    ReadyTicket* readyTicket = new (std::nothrow) ReadyTicket();
    if (readyTicket == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
    readyTicket->isReady = buffer[0] == 1;
    readyTicket->idxRead = ((uint64_t)buffer[8] << 56U) | ((uint64_t)buffer[7] << 48U) | ((uint64_t)buffer[6] << 40U) | ((uint64_t)buffer[5] << 32U) | ((uint64_t)buffer[4] << 24U) | ((uint64_t)buffer[3] << 16U) | ((uint64_t)buffer[2] << 8U) | buffer[1];
    readyTicket->maskRead = ((uint32_t)buffer[12] << 24U) | ((uint32_t)buffer[11] << 16U) | ((uint32_t)buffer[10]) << 8U | buffer[9];
    readyTicket->idxWrite = ((uint64_t)buffer[20] << 56U) | ((uint64_t)buffer[19] << 48U) | ((uint64_t)buffer[18] << 40U) | ((uint64_t)buffer[17] << 32U) | ((uint64_t)buffer[16] << 24U) | ((uint64_t)buffer[15] << 16U) | ((uint64_t)buffer[14] << 8U) | buffer[13];

    result.data = readyTicket;
    result.errorCode = Error::Nil;
    return result;
}
