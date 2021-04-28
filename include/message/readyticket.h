#ifndef _MESSAGE_READYTICKET_H_
#define _MESSAGE_READYTICKET_H_

#include <stdint.h>
#include <message/result.h>

class ReadyTicket {
private:
    bool isReady;
    uint64_t idxRead;
    uint32_t maskRead;
    uint64_t idxWrite;

public:
    bool getIsReady();
    uint64_t getIdxRead();
    uint32_t getMaskRead();
    uint64_t getIdxWrite();

    static Result<ReadyTicket *> parseBytes(uint8_t *buffer, uint8_t sizeBuffer);
};

#endif
