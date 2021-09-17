#ifndef ENTITY_SPIN_LOCK_H
#define ENTITY_SPIN_LOCK_H

#include <FreeRTOS.h>
#include <freertos/portmacro.h>

class SpinLock {
private:
    using spin_lock = portMUX_TYPE;
    spin_lock core;

public:
    SpinLock();
    SpinLock(SpinLock&& other) = delete;
    SpinLock(const SpinLock& other) = delete;
    ~SpinLock();

    SpinLock& operator=(SpinLock&& lock) = delete;
    SpinLock& operator=(const SpinLock& lock) = delete;

    void lock();
    void unlock();
};

#endif // !ENTITY_SPIN_LOCK_H