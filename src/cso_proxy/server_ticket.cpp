#include "cso_proxy/server_ticket.h"

ServerTicket::ServerTicket() noexcept
  : hubIP{ "" },
    ticketID{ 0 },
    ticketBytes{},
    serverSecretKey{} {}

ServerTicket::ServerTicket(std::string&& hubIP, uint16_t hubPort, uint16_t ticketID, Array<uint8_t>&& ticketBytes, Array<uint8_t>&& serverSecretKey)  noexcept
  : hubIP{ std::forward<std::string>(hubIP) },
    hubPort{ hubPort },
    ticketID{ ticketID },
    ticketBytes{ std::forward<Array<uint8_t>>(ticketBytes) },
    serverSecretKey{ std::forward<Array<uint8_t>>(serverSecretKey) } {}

ServerTicket::ServerTicket(ServerTicket&& other) noexcept
  : hubIP{ std::move(other.hubIP) },
    hubPort{ std::move(other.hubPort) },
    ticketID{ std::move(other.ticketID) },
    ticketBytes{ std::move(other.ticketBytes) },
    serverSecretKey{ std::move(other.serverSecretKey) } {}

ServerTicket::ServerTicket(const ServerTicket& other)
  : hubIP{ other.hubIP },
    hubPort{ other.hubPort },
    ticketID{ other.ticketID },
    ticketBytes{ other.ticketBytes },
    serverSecretKey{ other.serverSecretKey }{}

ServerTicket::~ServerTicket() noexcept {}

ServerTicket ServerTicket::operator=(ServerTicket&& other) noexcept {
    this->hubIP = std::move(other.hubIP);
    this->hubPort = std::move(other.hubPort);
    this->ticketID = std::move(other.ticketID);
    this->ticketBytes = std::move(other.ticketBytes);
    this->serverSecretKey = std::move(other.serverSecretKey);
    other.ticketID = 0;
    return *this;
}

ServerTicket& ServerTicket::operator=(const ServerTicket& other) {
    this->hubIP = other.hubIP;
    this->hubPort = other.hubPort;
    this->ticketID = other.ticketID;
    this->ticketBytes = other.ticketBytes;
    this->serverSecretKey = other.serverSecretKey;
    return *this;
}