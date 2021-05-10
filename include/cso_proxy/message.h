#ifndef _CSO_PROXY_MESSAGE_H_
#define _CSO_PROXY_MESSAGE_H_

#include <memory>
#include <string>
#include <BigNumber.h>

// ServerKey is a group of server keys
struct ServerKey {
    BigNumber gKey;
    BigNumber nKey;
    BigNumber pubKey;

    ServerKey() = default;
    ServerKey(const char* gKey, const char* nKey, const char* pubKey)
        : gKey(gKey), nKey(nKey), pubKey(pubKey) {}
};

// ServerTicket is an activation ticket from the Hub server
struct ServerTicket {
    std::string hubAddress;
    std::shared_ptr<byte> ticketBytes;
    std::shared_ptr<byte> serverSecretKey;
    uint16_t ticketID;

    ServerTicket() = default;
    ServerTicket(
        const char* hubAddress, 
        byte* ticketBytes,
        byte* serverSecretKey,
        uint16_t ticketID
    ) : hubAddress(hubAddress),
        ticketBytes(ticketBytes),
        serverSecretKey(serverSecretKey),
        ticketID(ticketID) {}
};

#endif // _CSO_PROXY_MESSAGE_H_