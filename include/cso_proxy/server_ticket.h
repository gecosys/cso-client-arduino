#ifndef CSO_PROXY_SERVER_TICKET
#define CSO_PROXY_SERVER_TICKET

#include <string>
#include <cstdint>
#include "entity/array.hpp"

// ServerTicket is an activation ticket from the Hub server
struct ServerTicket {
    std::string hubIP;
    uint16_t hubPort;
    uint16_t ticketID;
    Array<uint8_t> ticketBytes;
    Array<uint8_t> serverSecretKey;

    ServerTicket() noexcept;
    ServerTicket(ServerTicket&& other) noexcept;
    ServerTicket(const ServerTicket& other);
    ServerTicket(
        std::string&& hubIP,
        uint16_t hubPort,
        uint16_t ticketID,
        Array<uint8_t>&& ticketBytes,
        Array<uint8_t>&& serverSecretKey) noexcept;
    ~ServerTicket() noexcept;

    ServerTicket operator=(ServerTicket&& other) noexcept;
    ServerTicket& operator=(const ServerTicket& other);
};

#endif // !CSO_PROXY_SERVER_TICKET