#ifndef _SYNCHRONIZATION_SPIN_LOCK_H_
#define _SYNCHRONIZATION_SPIN_LOCK_H_

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

    void lock();
    void unlock();
};

#endif //_SYNCHRONIZATION_SPIN_LOCK_H_