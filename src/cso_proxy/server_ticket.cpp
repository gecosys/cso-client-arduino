#include "cso_proxy/server_ticket.h"

ServerTicket::ServerTicket() noexcept
    : hubIP(),
      hubPort(0),
      ticketID(0),
      ticketBytes(nullptr),
      serverSecretKey(nullptr) {}

ServerTicket::ServerTicket(
    std::string&& hubIP,
    uint16_t hubPort,
    uint16_t ticketID,
    uint8_t* ticketBytes,
    uint8_t* serverSecretKey
) noexcept 
    : hubIP(std::forward<std::string>(hubIP)),
      hubPort(hubPort),
      ticketID(ticketID),
      ticketBytes(ticketBytes),
      serverSecretKey(serverSecretKey) {}