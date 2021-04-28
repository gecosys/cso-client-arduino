#include <message/readyticket.h>
#include <message/define.h>


bool ReadyTicket::getIsReady() {
    return this->isReady;
}

uint64_t ReadyTicket::getIdxRead() {
    return this->idxRead;
}

uint32_t ReadyTicket::getMaskRead() {
    return this->maskRead;
}

uint64_t ReadyTicket::getIdxWrite() {
    return this->idxWrite;
}

// ParseBytes converts bytes to ReadyTicket
// Flag is_ready: 1 byte
// Idx Read: 8 bytes
// Mark Read: 4 bytes
// Idx Write: 8 bytes
Result<ReadyTicket *> ReadyTicket::parseBytes(uint8_t *buffer, uint8_t sizeBuffer) {
    Result<ReadyTicket *> result;
    if (sizeBuffer != 21) {
        result.errorCode = ERROR_CODE_INVALID_BYTES;
        return result;
    }

    ReadyTicket *readyTicket = new ReadyTicket();
    readyTicket->isReady = buffer[0] == 1;
    readyTicket->idxRead = ((uint64_t)buffer[8] << 56) | ((uint64_t)buffer[7] << 48) | ((uint64_t)buffer[6] << 40) | ((uint64_t)buffer[5] << 32) | ((uint64_t)buffer[4] << 24) | ((uint64_t)buffer[3] << 16) | ((uint64_t)buffer[2] << 8) | buffer[1];
    readyTicket->maskRead = ((uint32_t)buffer[12] << 24) | ((uint32_t)buffer[11] << 16) | ((uint32_t)buffer[10]) << 8 | buffer[9];
    readyTicket->idxWrite = ((uint64_t)buffer[20] << 56) | ((uint64_t)buffer[19] << 48) | ((uint64_t)buffer[18] << 40) | ((uint64_t)buffer[17] << 32) | ((uint64_t)buffer[16] << 24) | ((uint64_t)buffer[15] << 16) | ((uint64_t)buffer[14] << 8) | buffer[13];

    result.data = readyTicket;
    result.errorCode = SUCCESS;
    return result;
}
