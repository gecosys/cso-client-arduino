#ifndef _CSO_COUNTER_H_
#define _CSO_COUNTER_H_

#include <atomic>
#include <memory>
#include "interface.h"
#include "utils/utils_safe.h"
#include "utils/utils_spin_lock.h"

class Counter : public ICounter {
private:
    // Esp32 doesn't support "std::atomic<uin64_t>"
    // See more at "https://github.com/espressif/esp-idf/issues/3163"
    SpinLock spin;
    uint64_t writeIndex;
    uint64_t minReadIndex;
    uint32_t maskReadBits;

public:
    static std::shared_ptr<ICounter> build(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits);

private:
    friend class Safe;
    Counter() = default;
    Counter(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits);

public:
    Counter(Counter&& other) = delete;
    Counter(const Counter& other) = delete;
    virtual ~Counter();

    uint64_t nextWriteIndex() noexcept;
    void markReadUnused(uint64_t index) noexcept;
    bool markReadDone(uint64_t index) noexcept;
};

#endif //_CSO_COUNTER_H_