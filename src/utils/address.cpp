#include <cstring>
#include <stdlib.h>
#include <esp32-hal-log.h>
#include "utils/address.h"

Address Address::parse(const char* address) noexcept {
    size_t length = strlen(address);
    uint8_t index = -1;
    for (uint8_t idx = length - 1; idx >= 0; --idx) {
        if (address[idx] == ':') {
            index = idx;
            break;
        }
    }

    Address addr;
    if (index == -1) {
        return addr;
    }

    addr.m_ip.reset(new (std::nothrow) char[index + 1]);
    if (addr.m_ip == nullptr) {
        return addr;
    }
    memcpy(addr.m_ip.get(), address, index);
    addr.m_ip.get()[index] = '\0';
    addr.m_port = atoi(address + index + 1);
    return addr;
}

Address::Address() noexcept
    : m_ip(nullptr),
      m_port(0)  {}

Address::Address(Address&& rhs) noexcept
    : m_ip(nullptr),
      m_port(0) {
    this->m_port = rhs.m_port;
    this->m_ip.swap(rhs.m_ip);
}

Address::Address(const Address& rhs)
    : m_ip(nullptr),
      m_port(0) {
    size_t length = strlen(rhs.m_ip.get());
    this->m_ip.reset(new (std::nothrow) char[length + 1]);
    if (this->m_ip == nullptr) {
        log_e("Not enought memory for allocating");
        return;
    }
    this->m_port = rhs.m_port;
    strcpy(this->m_ip.get(), rhs.m_ip.get());
}

Address& Address::operator=(const Address& rhs) {
    size_t length = strlen(rhs.m_ip.get());
    char* temp = new (std::nothrow) char[length + 1];
    if (temp == nullptr) {
        log_e("Not enought memory for allocating");
        return *this;
    }

    this->m_ip.reset(temp);
    this->m_port = rhs.m_port;
    strcpy(this->m_ip.get(), rhs.m_ip.get());
    return *this;
}

Address& Address::operator=(Address&& rhs) noexcept {
    this->m_port = rhs.m_port;
    this->m_ip.swap(rhs.m_ip);
    return *this;
}

const char* Address::ip() const noexcept {
    return this->m_ip.get();
}

uint16_t Address::port() const noexcept {
    return this->m_port;
}