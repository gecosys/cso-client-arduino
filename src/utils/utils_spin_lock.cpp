#include "utils/utils_spin_lock.h"

SpinLock::SpinLock() {
    vPortCPUInitializeMutex(&this->core);
}

SpinLock::~SpinLock() {}

void SpinLock::lock() {
    vTaskEnterCritical(&this->core);
}

void SpinLock::unlock() {
    vTaskExitCritical(&this->core);
}