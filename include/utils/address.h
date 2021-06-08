#ifndef _UTILS_ADDRESS_H_
#define _UTILS_ADDRESS_H_

#include <memory>
#include <cstdint>

class Address {
private:
    std::unique_ptr<char> m_ip;
    uint16_t m_port;

public:
    static Address parse(const char* address) noexcept;

public:
    Address() noexcept;
    Address(Address&& rhs) noexcept;
    Address(const Address& rhs);

    Address& operator=(const Address& rhs);
    Address& operator=(Address&& rhs) noexcept;

    const char* ip() const noexcept;
    uint16_t port() const noexcept;
};

#endif //_UTILS_ADDRESS_H_