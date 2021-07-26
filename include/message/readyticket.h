#ifndef MESSAGE_READYTICKET_H
#define MESSAGE_READYTICKET_H

#include <tuple>
#include <memory>
#include <cstdint>
#include "entity/array.h"
#include "error/error.h"

class ReadyTicket {
private:
    bool isReady;
    uint32_t maskRead;
    uint64_t idxRead;
    uint64_t idxWrite;

public:
    ReadyTicket() noexcept;
    ~ReadyTicket() noexcept;

    bool getIsReady() noexcept;
    uint64_t getIdxRead() noexcept;
    uint32_t getMaskRead() noexcept;
    uint64_t getIdxWrite() noexcept;

    static std::tuple<Error::Code, std::unique_ptr<ReadyTicket>> parseBytes(const Array<uint8_t>& data) noexcept;
};

#endif // !MESSAGE_READYTICKET_H
