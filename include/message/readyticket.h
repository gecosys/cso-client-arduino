#ifndef _MESSAGE_READYTICKET_H_
#define _MESSAGE_READYTICKET_H_

#include <cstdint>
#include "message/result.h"

class ReadyTicket {
private:
    bool isReady;
    uint32_t maskRead;
    uint64_t idxRead;
    uint64_t idxWrite;

public:
    bool getIsReady() noexcept;
    uint64_t getIdxRead() noexcept;
    uint32_t getMaskRead() noexcept;
    uint64_t getIdxWrite() noexcept;

    static Result<ReadyTicket*> parseBytes(uint8_t* buffer, uint8_t sizeBuffer) noexcept;
};

#endif
