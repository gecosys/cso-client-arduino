#ifndef _CSO_PROXY_SERVER_TICKET_H_
#define _CSO_PROXY_SERVER_TICKET_H_

#include <memory>
#include <string>

// ServerTicket is an activation ticket from the Hub server
class ServerTicket {
public:
    std::string hubIP;
    uint16_t hubPort;
    uint16_t ticketID;
    std::shared_ptr<uint8_t> ticketBytes;
    std::shared_ptr<uint8_t> serverSecretKey;

public:
    ServerTicket() noexcept;
    ServerTicket(
        std::string&& hubIP,
        uint16_t hubPort,
        uint16_t ticketID,
        uint8_t* ticketBytes,
        uint8_t* serverSecretKey
    ) noexcept;

    // ServerTicket(ServerTicket&& other) noexcept {
    //   this->ticketID = other.ticketID;
    //   this->hubPort = other.hubPort;
    //   std::swap(this->hubIP, other.hubIP);
    //   std::swap(this->ticketBytes, other.ticketBytes);
    //   std::swap(this->serverSecretKey, other.serverSecretKey);
    // }
};

#endif // _CSO_PROXY_SERVER_TICKET_H_