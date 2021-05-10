#ifndef _COUNTER_INTERFACE_H_
#define _COUNTER_INTERFACE_H_

#include <cstdint>

class ICounter {
public:
    virtual uint64_t nextWriteIndex() noexcept = 0;
    virtual void markReadUnused(uint64_t index) noexcept = 0;
    virtual bool markReadDone(uint64_t index) noexcept = 0;
};

#endif //_COUNTER_INTERFACE_H_