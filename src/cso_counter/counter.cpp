#include <new>
#include "cso_counter/counter.h"

#define NUMBER_BITS 32

std::unique_ptr<ICounter> Counter::build(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits) {
    ICounter* obj = new (std::nothrow) Counter(writeIndex - 1, minReadIndex, maskReadBits);
    if (obj == nullptr) {
        throw std::runtime_error("[cso_counter/Counter::build(...)]Not enough memory to create object");
    }
    return std::unique_ptr<ICounter>(obj);
}

Counter::Counter(uint64_t writeIndex, uint64_t minReadIndex, uint32_t maskReadBits) noexcept
    : spin(),
      writeIndex(writeIndex - 1),
      minReadIndex(minReadIndex),
      maskReadBits(maskReadBits) {}

Counter::~Counter() noexcept {}

uint64_t Counter::nextWriteIndex() noexcept {
    this->spin.lock();
    uint64_t ret = this->writeIndex;
    this->writeIndex += 1;
    this->spin.unlock();
    return ret;
}

void Counter::markReadUnused(uint64_t index) noexcept {
    if (index < this->minReadIndex) {
        return;
    }
    if (index >= this->minReadIndex + NUMBER_BITS) {
        return;
    }
    this->maskReadBits &= ~(0x01U << (index - this->minReadIndex));
}

bool Counter::markReadDone(uint64_t index) noexcept {
    if (index < this->minReadIndex) {
        return false;
    }

    if (index >= this->minReadIndex + NUMBER_BITS) {
        this->minReadIndex += NUMBER_BITS;
        this->maskReadBits = 0;
    }
    
    uint32_t mask = 0x01U << (index - this->minReadIndex);
	if ((this->maskReadBits & mask) != 0) {
		return false;
	}
	this->maskReadBits |= mask;
	return true;
}