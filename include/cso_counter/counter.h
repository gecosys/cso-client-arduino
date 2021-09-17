#ifndef CSO_COUNTER_H
#define CSO_COUNTER_H

#include <atomic>
#include <memory>
#include "interface.h"
#include "entity/spinlock.h"

class Counter : public ICounter {
private:
    // Esp32 doesn't support "std::atomic<uin64_t>"
    // See more at "https://github.com/espressif/esp-idf/issues/3163"
    SpinLock spin;
    uint64_t writeIndex;
    uint64_t minReadIndex;
    uint32_t maskReadBits;

public:
    static std::unique_ptr<ICounter> build(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits);

private:
    Counter(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits) noexcept;

public:
    Counter() = delete;
    Counter(Counter&& other) = delete;
    Counter(const Counter& other) = delete;
    ~Counter() noexcept = default;

    Counter& operator=(Counter&& other) = delete;
    Counter& operator=(const Counter& other) = delete;

    uint64_t nextWriteIndex() noexcept;
    void markReadUnused(uint64_t index) noexcept;
    bool markReadDone(uint64_t index) noexcept;
};

#endif // !CSO_COUNTER_H